//
// Created by utnso on 13/05/20.
//

#include "entrenador.h"

extern t_log* default_logger;

static t_list* entrenadores;
static t_list* sem_entrenadores;
const int COSTO_INTERCAMBIO = 5;

static int calcular_distancia(t_coord*, t_coord*);
static void avanzar(t_entrenador*);
static void log_entrenador(void*);
static void log_pokemon(void*);
static void log_objetivo(void*);
static bool entrenador_is_full(t_entrenador*);
static t_list* get_disponibles();
static t_coord* get_ubicacion_objetivo_actual(t_entrenador*);
static bool en_camino(t_entrenador*);
static void realizar_intercambio(t_entrenador*);
static bool queda_quantum(int id);
static t_pokemon_mapeado* encontrar_pokemon_cercano(t_entrenador*, t_list*);
static t_list* get_objetivos_asignables();
static void consumir_quantum_restante(int id);
static void log_estado_objetivos(t_entrenador*);
static void log_algoritmo(t_entrenador* entrenador);
static void logs_movimiento_entrenador(t_entrenador* entrenador);
static void logs_atrapar(t_entrenador* entrenador);
static void logs_intercambio(t_entrenador* entrenador,t_entrenador* otro_entrenador, t_pokemon_capturado* entregado,t_pokemon_capturado* recibido);
static void logs_error_transicion(int, char*);

static t_info* create_info(){
    t_info* info = malloc(sizeof(t_info));

    info->ciclos_cpu_ejecutados = 0;
    info->ejecucion_parcial = 0;
    info->estimado_siguiente_rafaga = config_team->estimacion_inicial;
    info->ultima_ejecucion = config_team->estimacion_inicial;
    info->ultima_estimacion = config_team->estimacion_inicial;

    return info;
}

t_entrenador* entrenador_new(int id, t_list* objetivos, t_list* capturados, t_coord* posicion){
    t_entrenador* entrenador = malloc(sizeof(t_entrenador));

    entrenador->id = id;
    entrenador->objetivos = objetivos;
    entrenador->capturados = capturados;
    entrenador->posicion = posicion;
    entrenador->estado = NEW;
    entrenador->deadlock = false;
    entrenador->pokemon_buscado = NULL;
    entrenador->intercambio = NULL;
    entrenador->info = create_info();

    logs_transicion(entrenador, NEW);

    return entrenador;
}

void entrenador_init_list(t_list* nuevos_entrenadores) {
    sem_init(&sem_post_ejecucion, 0, 0);
    entrenadores = list_create();
    list_add_all(entrenadores, nuevos_entrenadores);
    void _init_semaforos(void* elem) {
        sem_t newSem;
        sem_init(&newSem, 0, 0);
        sem_t* sem = malloc(sizeof(sem_t));
        memcpy(sem, &newSem, sizeof(sem_t));
        list_add(sem_entrenadores, sem);
    }
    sem_entrenadores = list_create();
    pthread_mutex_init(&mx_entrenadores, NULL);
    list_iterate(entrenadores, (void*)_init_semaforos);
}

t_entrenador* entrenador_get_libre_mas_cercano(t_coord* posicion_buscada) {
    t_list* disponibles = get_disponibles();
    int menor = 10000, entrenador_mas_cercano = -1;
    for (int i=0 ; i<list_size(disponibles) ; i++) {
        t_entrenador* entrenador = list_get(disponibles, i);
        int actual = calcular_distancia(entrenador->posicion, posicion_buscada);
        if (actual < menor) {
            menor = actual;
            entrenador_mas_cercano = entrenador->id;
        }
    }
    list_destroy(disponibles);

    if (entrenador_mas_cercano < 0) {
        return NULL;
    }

    t_entrenador* e = (t_entrenador*) list_get(entrenadores, entrenador_mas_cercano);
    log_debug(default_logger, "Se planifica al entrenador #%d en (%d, %d) hacia la posicion (%d, %d)",
            e->id, e->posicion->x, e->posicion->y, posicion_buscada->x, posicion_buscada->y);

    return e;
}

void entrenador_otorgar_ciclos_ejecucion(t_entrenador* entrenador, int cant) {
    logs_transicion(entrenador, EXECUTE);
    entrenador->estado = EXECUTE;
    for (int i = 0 ; i < cant ; i++) {
        sem_post(list_get(sem_entrenadores, entrenador->id));
    }
}

void entrenador_abortar_ejecucion() {
    bool _is_executing(void* e) {
        return ((t_entrenador*)e)->estado == EXECUTE;
    }
    t_entrenador* en_ejecucion = list_find(entrenadores, (void*)_is_executing);
    if(en_ejecucion) {
        consumir_quantum_restante(en_ejecucion->id);
    }
}

void entrenador_execute(t_entrenador* e) {
    log_estado_objetivos(e);
    while(1) {
        if (!queda_quantum(e->id)) {
            if (e->estado != NEW && e->estado != READY) {
                sem_post(&sem_post_ejecucion);
            }
            sem_wait(list_get(sem_entrenadores, e->id));
        }

        sleep(config_team->retardo_ciclo_cpu);
        e->info->ciclos_cpu_ejecutados++;
        e->info->ejecucion_parcial++; //solo sirve para SJF-CD

        if (en_camino(e)) {
            avanzar(e);
        } else if (e->deadlock) {
            log_debug(default_logger, "Realizando intercambio...");
            if (--e->intercambio->remaining_intercambio < 1) {
                realizar_intercambio(e);
                consumir_quantum_restante(e->id);
            }
        } else {
            logs_transicion(e, BLOCKED_WAITING);
            e->estado = BLOCKED_WAITING;
            log_debug(default_logger, "Entrenador #%d paso a estado BLOCKED_WAITING", e->id);
            logs_atrapar(e);
            bool enviado = enviar_catch_pokemon(e->id, e->pokemon_buscado);
            if (!enviado) {
                log_debug(default_logger, "Entrenador #%d capturo automaticamente a %s", e->id, e->pokemon_buscado->pokemon);
                entrenador_concretar_captura(e, e->pokemon_buscado->pokemon, e->pokemon_buscado->ubicacion);
                entrenador_verificar_objetivos(e);
            }
            consumir_quantum_restante(e->id);
        }
    }
}

t_entrenador* entrenador_asignar(char* pokemon) {
    t_list* entrenadores = get_disponibles();
    t_list* especies = list_create();
    list_add(especies, pokemon);

    int minima_distancia = 999999;
    t_pokemon_mapeado* objetivo = NULL;
    t_entrenador* entrenador = NULL;
    void _calcular_minima_distancia(void* e) {
        t_entrenador* this_entrenador = (t_entrenador*) e;
        t_pokemon_mapeado* cercano = encontrar_pokemon_cercano(this_entrenador, especies);
        int d = calcular_distancia(this_entrenador->posicion, cercano->ubicacion);
        if (d < minima_distancia) {
            minima_distancia = d;
            entrenador = this_entrenador;
            objetivo = cercano;
        }
    }
    list_iterate(entrenadores, (void*)_calcular_minima_distancia);

    list_destroy(entrenadores);
    list_destroy(especies);

    if (objetivo == NULL) return NULL;

    entrenador->pokemon_buscado = objetivo;

    return entrenador;
}

void entrenador_concretar_captura(t_entrenador* e, char* pokemon, t_coord* ubicacion) {
    bool _find(void* obj) {
        t_pokemon_objetivo* objetivo = (t_pokemon_objetivo*) obj;
        return !objetivo->fue_capturado && string_equals_ignore_case(objetivo->nombre, pokemon);
    }
    t_pokemon_objetivo* objetivo = list_find(e->objetivos, (void*)_find);
    if (objetivo) {
        objetivo->fue_capturado = true;
    }

    t_pokemon_capturado* captura = malloc(sizeof(t_pokemon_capturado));
    captura->nombre = string_duplicate(e->pokemon_buscado->pokemon);
    captura->es_objetivo = objetivo != NULL;
    list_add(e->capturados, captura);

    objetivos_capturado(e->pokemon_buscado->pokemon);
    pokemon_sacar_del_mapa(e->pokemon_buscado->pokemon, e->pokemon_buscado->ubicacion);
    e->pokemon_buscado = NULL; // es el mismo puntero que el del mapa y ya se libera en la funcion de arriba

    log_estado_objetivos(e);
}

t_entrenador* entrenador_get(int id) {
    return list_get(entrenadores, id);
}

t_list* entrenador_get_all() {
    return entrenadores;
}

int entrenador_get_count() {
    return list_size(entrenadores);
}

int entrenador_calcular_remaining(t_entrenador* e) {
    if (e->pokemon_buscado == NULL && e->intercambio == NULL) {
        log_error(default_logger, "Calculando remaining para entrenador #%d sin objetivo", e->id);
        return -1;
    }
    return calcular_distancia(e->posicion, get_ubicacion_objetivo_actual(e)) + (e->deadlock ? 5 : 1);
}

static bool entrenador_is_full(t_entrenador* e) {
    return list_size(e->capturados) == list_size(e->objetivos);
}

void entrenador_verificar_objetivos(t_entrenador* e) {
    if (!entrenador_is_full(e)) {
        if (e->estado == NEW) return;
        logs_transicion(e, BLOCKED_IDLE);
        e->estado = BLOCKED_IDLE;
        log_debug(default_logger, "Entrenador #%d paso a estado BLOCKED_IDLE", e->id);
        return;
    }

    if (entrenador_cumplio_objetivos(e)) {
        logs_transicion(e, EXIT);
        e->estado = EXIT;
        log_debug(default_logger, "Entrenador #%d paso a estado EXIT", e->id);
    } else {
        logs_transicion(e, BLOCKED_FULL);
        e->estado = BLOCKED_FULL;
        log_debug(default_logger, "Entrenador #%d paso a estado BLOCKED_FULL", e->id);
    }
}

int entrenador_cantidad_asignado_a(char* pokemon) {
    int count = 0;
    void _count(void* item) {
        t_entrenador* e = (t_entrenador*) item;
        if (e->pokemon_buscado != NULL && string_equals_ignore_case(e->pokemon_buscado->pokemon, pokemon)) count++;
    }
    list_iterate(entrenadores, (void*)_count);

    return count;
}

bool entrenador_cumplio_objetivos(t_entrenador* e) {
    bool _cumple_objetivo(void* item) {
        t_pokemon_objetivo* obj = (t_pokemon_objetivo*) item;
        return obj->fue_capturado;
    }
    return list_all_satisfy(e->objetivos, (void*)_cumple_objetivo);
}

void entrenador_asignar_objetivo_a(t_entrenador* e) {
    t_list* objetivos_asignables = get_objetivos_asignables();

    if (list_size(objetivos_asignables) > 0) {
        e->pokemon_buscado = encontrar_pokemon_cercano(e, objetivos_asignables);
    }

    list_destroy(objetivos_asignables);
}

static t_list* get_objetivos_asignables() {
    t_list* objetivos = objetivos_get_especies_pendientes();
    t_list* objetivos_localizados = pokemon_filtrar_especies_encontradas(objetivos);

    // esta logica ya estaba implementada en subscripcion.c pero se duplica por simplicidad
    bool _es_asignable(void* especie) {
        char* pokemon = (char*) especie;

        int capturas_pendientes = objetivos_cantidad_pendientes(pokemon);
        if (capturas_pendientes == 0) return false;

        int entrenadores_asignados = entrenador_cantidad_asignado_a(pokemon);

        return entrenadores_asignados < capturas_pendientes;
    }
    pthread_mutex_lock(&mx_atrapados);
    t_list* objetivos_asignables = list_filter(objetivos_localizados, (void*)_es_asignable);
    pthread_mutex_unlock(&mx_atrapados);

    list_destroy(objetivos);
    list_destroy(objetivos_localizados);

    return objetivos_asignables;
}

static t_pokemon_mapeado* encontrar_pokemon_cercano(t_entrenador* e, t_list* pokemons) {
    int menor_distacia = 10000;
    t_pokemon_mapeado* objetivo = NULL;

    void _procesar(void* pokemon) {
        char* especie = (char*) pokemon;
        t_list* ubicaciones = pokemon_get(especie);

        if (ubicaciones == NULL) return;

        void _comparar_distancia(void* p) {
            t_pokemon_mapeado* pokemon = (t_pokemon_mapeado*) p;
            int distancia = calcular_distancia(e->posicion, pokemon->ubicacion);
            if (distancia < menor_distacia) {
                menor_distacia = distancia;
                objetivo = pokemon;
            }
        }

        list_iterate(ubicaciones, (void*)_comparar_distancia);
    }

    list_iterate(pokemons, (void*)_procesar);

    return objetivo;
}

static void realizar_intercambio(t_entrenador* entrenador) {
    t_intercambio* i = entrenador->intercambio;
    t_entrenador* otro_entrenador = entrenador_get(i->id_otro_entrenador);

    bool _es_recibido(void* item) {
        t_pokemon_capturado* pokemon = (t_pokemon_capturado*) item;
        return !pokemon->es_objetivo && string_equals_ignore_case(pokemon->nombre, i->recibo_pokemon);
    }
    t_pokemon_capturado* recibo = list_remove_by_condition(otro_entrenador->capturados, (void*)_es_recibido);

    bool _es_entregado(void* item) {
        t_pokemon_capturado* pokemon = (t_pokemon_capturado*) item;
        return !pokemon->es_objetivo && string_equals_ignore_case(pokemon->nombre, i->entrego_pokemon);
    }
    t_pokemon_capturado* entrego = list_remove_by_condition(entrenador->capturados, (void*)_es_entregado);

    if (recibo == NULL || entrego == NULL) {
        log_error(default_logger, "Error al realizar intercambio: no se encontro alguno de los pokemon para intercambiar");
        exit(EXIT_FAILURE);
    }

    list_add(entrenador->capturados, recibo);
    list_add(otro_entrenador->capturados, entrego);

    bool _mi_objetivo(void* item) {
        t_pokemon_objetivo* objetivo = (t_pokemon_objetivo*) item;
        return !objetivo->fue_capturado && string_equals_ignore_case(objetivo->nombre, recibo->nombre);
    }
    t_pokemon_objetivo* mi_objetivo = list_find(entrenador->objetivos, _mi_objetivo);

    bool _otro_objetivo(void* item) {
        t_pokemon_objetivo* objetivo = (t_pokemon_objetivo*) item;
        return !objetivo->fue_capturado && string_equals_ignore_case(objetivo->nombre, entrego->nombre);
    }
    t_pokemon_objetivo* otro_objetivo = list_find(otro_entrenador->objetivos, _otro_objetivo);

    recibo->es_objetivo = mi_objetivo->fue_capturado = true;
    if (otro_objetivo != NULL) {
        entrego->es_objetivo = otro_objetivo->fue_capturado = true;
    }

    entrenador_verificar_objetivos(entrenador);
    entrenador_verificar_objetivos(otro_entrenador);

    logs_intercambio(entrenador, otro_entrenador, entrego, recibo);
    log_debug(default_logger, "Se realizo un intercambio entre entrenador #%d (%s) y entrenador #%d (%s)",
            entrenador->id, entrego->nombre, otro_entrenador->id, recibo->nombre);

    log_estado_objetivos(entrenador);
    log_estado_objetivos(otro_entrenador);
}

t_list* entrenador_calcular_pokemon_faltantes(t_entrenador* e) {
    bool _no_capturado(void* item) {
        t_pokemon_objetivo* objetivo = (t_pokemon_objetivo*) item;
        return !objetivo->fue_capturado;
    }
    return list_filter(e->objetivos, (void*)_no_capturado);
}

t_list* entrenador_calcular_pokemon_sobrantes(t_entrenador* e) {
    bool _sobrante(void* item) {
        t_pokemon_capturado* capturado = (t_pokemon_capturado*) item;
        return !capturado->es_objetivo;
    }
    return list_filter(e->capturados, (void*)_sobrante);
}

t_list* entrenador_get_bloqueados(int id) {
    bool _is_waiting_deadlock(void* elem) {
        t_entrenador* e = (t_entrenador*) elem;
        return e->id != id && e->estado == BLOCKED_FULL && !e->deadlock;
    }
    return list_filter(entrenadores, (void*)_is_waiting_deadlock);
}

static void avanzar(t_entrenador* e) {
    t_coord* actual = e->posicion;
    t_coord* objetivo = get_ubicacion_objetivo_actual(e);
    if (actual->x != objetivo->x) {
        if (actual->x > objetivo->x) {
            actual->x--;
        } else {
            actual->x++;
        }
    } else if (actual->y != objetivo->y) {
        if (actual->y > objetivo->y) {
            actual->y--;
        } else {
            actual->y++;
        }
    }
    logs_movimiento_entrenador(e);
    log_debug(default_logger, "Entrenador #%d se movio a la posicion (%d, %d)", e->id, actual->x, actual->y);
}

static t_list* get_disponibles() {
    bool _is_disponible(void* elemento) {
        t_entrenador* entrenador = (t_entrenador*) elemento;
        return entrenador->estado == NEW || entrenador->estado == BLOCKED_IDLE;
    }
    return list_filter(entrenadores, (void*)_is_disponible);
}

static int calcular_distancia(t_coord* entrenador, t_coord* destino) {
    return (int) fabs((double)destino->x - (double)entrenador->x) + fabs((double)destino->y - (double)entrenador->y);
}

static t_coord* get_ubicacion_objetivo_actual(t_entrenador* e) {
    return e->deadlock ? e->intercambio->ubicacion : e->pokemon_buscado->ubicacion;
}

static bool en_camino(t_entrenador* e) {
    return calcular_distancia(e->posicion, get_ubicacion_objetivo_actual(e)) > 0;
}

void log_entrenadores(t_list* entrenadores){
    log_debug(default_logger, "Entrenadores:");
    list_iterate(entrenadores, (void*)log_entrenador);
}

static void log_entrenador(void* element){
  t_entrenador* entrenador = element;
  log_debug(default_logger, "       Entrenador:");
  log_debug(default_logger, "               *Posicision: x=%i ; y=%i", entrenador->posicion->x, entrenador->posicion->y);
  log_debug(default_logger, "               *Objetivos: ");
  list_iterate(entrenador->objetivos, (void*)log_objetivo);
  log_debug(default_logger, "               *Atrapados: ");
  list_iterate(entrenador->capturados, (void*)log_pokemon);
}

static void log_pokemon(void* elemento){
    t_pokemon_capturado* pokemon = (t_pokemon_capturado*) elemento;
    log_debug(default_logger, "                         -%s (%s)", pokemon->nombre, pokemon->es_objetivo ? "OBJETIVO" : "INTERCAMBIO");
}

static void log_objetivo(void* elemento){
    t_pokemon_objetivo* pokemon = (t_pokemon_objetivo*) elemento;
    log_debug(default_logger, "                         -%s (%s)", pokemon->nombre, pokemon->fue_capturado ? "CAPTURADO" : "BUSCADO");
}

static bool queda_quantum(int id) {
    sem_t* sem = list_get(sem_entrenadores, id);
    return sem_trywait(sem) == 0;
}

static void consumir_quantum_restante(int id) {
    while (queda_quantum(id));
}

static void log_algoritmo(t_entrenador* entrenador){
    if (string_equals_ignore_case(config_team->algoritmo_planificacion, "SJF-CD")) {
        log_info(logger, "Entrenador: %d pasa a READY desde EXECUTE por DESALOJO", entrenador->id);
    }
    else{
        log_info(logger, "Entrenador: %d pasa a READY desde EXECUTE por FIN DE QUANTUM", entrenador->id);
    }
}

void logs_transicion(t_entrenador* entrenador, t_estado nuevoEstado){
    switch(nuevoEstado){
        case NEW:
            log_info(logger, "Entrenador: %d pasa a NEW por inicializacion", entrenador->id);
            break;

        case READY:
            switch(entrenador->estado){
            case NEW:
                log_info(logger, "Entrenador: %d pasa a READY desde NEW por aparicion de nuevo objetivo", entrenador->id);
                break;

            case EXECUTE:
                log_algoritmo(entrenador);
                break;

            case BLOCKED_IDLE:
                log_info(logger, "Entrenador: %d pasa a READY desde BLOCKED_IDLE por aparicion de nuevo objetivo", entrenador->id);
                break;

            case BLOCKED_FULL:
                log_info(logger, "Entrenador: %d pasa a READY desde BLOCKED_FULL por resolucion de Deadlock", entrenador->id);
                break;

            default:
                logs_error_transicion(entrenador->estado, "READY");
                break;
            }
            break;

        case BLOCKED_IDLE:
            switch(entrenador->estado){
            case EXECUTE:
                log_info(logger, "Entrenador: %d pasa a BLOCKED_IDLE desde EXECUTE esperando nuevo objetivo", entrenador->id);
                break;

            case BLOCKED_WAITING:
                log_info(logger, "Entrenador: %d pasa a BLOCKED_IDLE desde BLOCKED_WAITING por confirmacion del CAUGHT", entrenador->id);
                break;

            default:
                logs_error_transicion(entrenador->estado, "BLOCKED_IDLE");
                break;
            }
            break;

        case BLOCKED_WAITING:
            log_info(logger, "Entrenador: %d pasa a BLOCKED_WAITING desde EXECUTE por espera de confimacion del CATCH", entrenador->id);
            break;

        case BLOCKED_FULL:
            switch(entrenador->estado){
                case EXECUTE:
                    log_info(logger, "Entrenador: %d pasa a BLOCKED_FULL desde EXECUTE por llegada al limite de capturas despues de realizar un intercambio", entrenador->id);
                    break;

                case BLOCKED_WAITING:
                    log_info(logger, "Entrenador: %d pasa a BLOCKED_FULL desde BLOCKED_WAITING por llegada al limite de capturas", entrenador->id);
                    break;

                case BLOCKED_FULL:
                    log_info(logger, "Entrenador: %d sigue en BLOCKED_FULL despues de realizar un intercambio", entrenador->id);
                    break;

                case NEW:
                    log_info(logger, "Entrenador: %d pasa a BLOCKED_FULL desde NEW por iniciar sin espacio para nuevas capturas", entrenador->id);
                    break;

                default:
                    logs_error_transicion(entrenador->estado, "BLOCKED_FULL");
                    break;
            }
            break;

        case EXIT:
        switch(entrenador->estado){
            case BLOCKED_WAITING:
                log_info(logger, "Entrenador: %d pasa a EXIT desde BLOCKED_WAITING por finalizacion de sus objetivos", entrenador->id);
            break;

            case BLOCKED_FULL:
                log_info(logger, "Entrenador: %d pasa a EXIT desde BLOCKED_FULL por finalizacion de sus objetivos post intercambios", entrenador->id);
                break;

            case EXECUTE:
                log_info(logger, "Entrenador: %d pasa a EXIT desde EXECUTE por resolucion del intercambio y finalizacion de sus objetivos", entrenador->id);
                break;

            case NEW:
                log_info(logger, "Entrenador: %d pasa a EXIT desde NEW por iniciar con sus objetivos cumplidos", entrenador->id);
                break;

            default:
                logs_error_transicion(entrenador->estado, "EXIT");
            break;
        }
        break;

        case EXECUTE:
            log_info(logger, "Entrenador: %d pasa a EXECUTE desde READY por seleccion en cola", entrenador->id);
            break;

        default:
            logs_error_transicion(entrenador->estado, "");
            break;
    }
}

static void logs_movimiento_entrenador(t_entrenador* entrenador){
    log_info(logger, "Entrenador: %d se movio a la posicion (%d, %d)", entrenador->id, entrenador->posicion->x, entrenador->posicion->y);
}

static void logs_atrapar(t_entrenador* entrenador){
    log_info(logger, "Entrenador: %d intenta capturar a %s en la posicion (%d, %d)",
            entrenador->id, entrenador->pokemon_buscado->pokemon, entrenador->pokemon_buscado->ubicacion->x, entrenador->pokemon_buscado->ubicacion->y);
}

static void logs_intercambio(t_entrenador* entrenador,t_entrenador* otro_entrenador, t_pokemon_capturado* entregado,t_pokemon_capturado* recibido){
    log_info(logger, "Se realizo un intercambio entre Entrenador: %d -> (%s) y Entrenador: %d -> (%s)", entrenador->id, entregado->nombre, otro_entrenador->id, recibido->nombre);
}

void logs_inicio_deteccion_deadlock(){
    log_info(logger, "Inicio del algoritmo de deteccion de Deadlock");
}

void logs_deadlock(bool existe_deadlock){
    log_info(logger, "Resultado: %s deadlock", existe_deadlock ? "Existe" : "No existe");
}

static void logs_error_transicion(int actual, char* nuevo){
    log_error(logger, "Error en el cambio de cola de planificacion (%d > %s)", actual, nuevo);
}

static void log_estado_objetivos(t_entrenador* e) {
    char* str = string_new();
    void _concat_nombres(void* pokemon) {
        if (string_length(str) > 0) {
            string_append(&str, ", ");
        }
        t_pokemon_objetivo* p = (t_pokemon_objetivo*) pokemon;
        string_append(&str, p->nombre);
        string_append(&str, p->fue_capturado ? "(+)" : "(-)");
    }
    list_iterate(e->objetivos, (void*)_concat_nombres);
    char* objetivos = str;
    str = string_new();
    list_iterate(e->capturados, (void*)_concat_nombres);
    char* capturas = str;
    log_debug(default_logger, "Entrenador #%d\n\tObjetivos: [%s]\n\tCapturas: [%s]", e->id, objetivos, capturas);
    free(objetivos);
    free(capturas);
}
