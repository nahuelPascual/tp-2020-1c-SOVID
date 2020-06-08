//
// Created by utnso on 13/05/20.
//

#include "entrenador.h"

extern t_log* default_logger;

static t_list* entrenadores;
static t_list* sem_entrenadores;

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

static t_info* create_info(){
    t_info* info = malloc(sizeof(t_info));

    info->ciclos_cpu_ejecutados = 0;
    info->deadlocks = 0;
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
    list_iterate(entrenadores, (void*)_init_semaforos);
}

t_entrenador* entrenador_get_libre_mas_cercano(t_coord* posicion_buscada) {
    t_list* disponibles = get_disponibles();
    int menor = 10000, entrenador_mas_cercano;
    for (int i=0 ; i<list_size(disponibles) ; i++) {
        t_entrenador* entrenador = list_get(disponibles, i);
        int actual = calcular_distancia(entrenador->posicion, posicion_buscada);
        if (actual < menor) {
            menor = actual;
            entrenador_mas_cercano = entrenador->id;
        }
    }
    list_destroy(disponibles);
    t_entrenador* e = (t_entrenador*) list_get(entrenadores, entrenador_mas_cercano);
    log_debug(default_logger, "Se planifica al entrenador #%d en (%d, %d) hacia la posicion (%d, %d)",
            e->id, e->posicion->x, e->posicion->y, posicion_buscada->x, posicion_buscada->y);
    return e;
}

void entrenador_otorgar_ciclos_ejecucion(t_entrenador* entrenador, int cant) {
    for (int i = 0 ; i < cant ; i++) {
        sem_post(list_get(sem_entrenadores, entrenador->id));
    }
}

void entrenador_execute(t_entrenador* e) {
    while(1) {
        if (!queda_quantum(e->id)) {
            if (e->estado != NEW && e->estado != READY) {
                sem_post(&sem_post_ejecucion);
            }
            sem_wait(list_get(sem_entrenadores, e->id));
        }

        log_debug(default_logger, "Ejecutando entrenador #%d", e->id);
        e->estado = EXECUTE;
        sleep(config_team->retardo_ciclo_cpu);
        e->info->ciclos_cpu_ejecutados++;

        if (en_camino(e)) {
            avanzar(e);
        } else if (e->deadlock) {
            if (--e->intercambio->remaining_intercambio < 1) {
                realizar_intercambio(e);
                while (queda_quantum(e->id)); // consumir quantum restante
            }
        } else {
            e->estado = BLOCKED_WAITING;
            log_debug(default_logger, "Entrenador #%d paso a estado BLOCKED_WAITING", e->id);
            bool enviado = enviar_catch_pokemon(e->id, e->pokemon_buscado);
            if (!enviado) {
                log_debug(default_logger, "Entrenador #%d capturo automaticamente a %s", e->id, e->pokemon_buscado->pokemon);
                entrenador_concretar_captura(e, e->pokemon_buscado->pokemon, e->pokemon_buscado->ubicacion);
                entrenador_verificar_objetivos(e);
            }
            while (queda_quantum(e->id)); // consumir quantum restante
        }
    }
}

void entrenador_concretar_captura(t_entrenador* e, char* pokemon, t_coord* ubicacion) {
    list_add(e->capturados, string_duplicate(e->pokemon_buscado->pokemon));
    objetivos_descontar_requeridos(e->pokemon_buscado->pokemon);
    pokemon_sacar_del_mapa(e->pokemon_buscado->pokemon, e->pokemon_buscado->ubicacion);
    e->pokemon_buscado = NULL; // es el mismo puntero que el del mapa y ya se libera en la funcion de arriba
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
        e->estado = BLOCKED_IDLE;
        log_debug(default_logger, "Entrenador #%d paso a estado BLOCKED_IDLE", e->id);
        return;
    }

    if (entrenador_cumplio_objetivos(e)) {
        e->estado = EXIT;
        log_debug(default_logger, "Entrenador #%d paso a estado EXIT", e->id);
    } else {
        e->estado = BLOCKED_FULL;
        log_debug(default_logger, "Entrenador #%d paso a estado BLOCKED_FULL", e->id);
    }
}

bool entrenador_asignado_a(t_pokemon_mapeado* pokemon) {
    bool _persigue_a(void* item) {
        t_entrenador* e = (t_entrenador*) item;
        return e->pokemon_buscado != NULL && string_equals_ignore_case(e->pokemon_buscado->pokemon, pokemon->pokemon);
    }
    return list_any_satisfy(entrenadores, (void*)_persigue_a);
}

bool entrenador_cumplio_objetivos(t_entrenador* e) {
    bool _cumple_objetivo(void* item) {
        t_pokemon_objetivo* obj = (t_pokemon_objetivo*) item;
        return obj->fue_capturado;
    }
    return list_all_satisfy(e->objetivos, (void*)_cumple_objetivo);
}

void entrenador_asignar_objetivo(t_entrenador* e) {
    t_list* objetivos_pendientes = pokemon_filtrar_especies_encontradas(objetivos_get_especies());

    bool _es_asignable(void* especie) {
        char* pokemon = (char*) especie;
        bool _persigue_a(void* item) {
            t_entrenador* e = (t_entrenador*) item;
            return e->pokemon_buscado != NULL && string_equals_ignore_case(e->pokemon_buscado->pokemon, pokemon);
        }
        return !list_any_satisfy(entrenadores, (void*)_persigue_a);
    }
    objetivos_pendientes = list_filter(objetivos_pendientes, (void*)_es_asignable);

    if (list_size(objetivos_pendientes) == 0) return; // no hay en el mapa ninguno de los objetivos del entrenador

    e->pokemon_buscado = encontrar_pokemon_cercano(e, objetivos_pendientes);
}

static t_pokemon_mapeado* encontrar_pokemon_cercano(t_entrenador* e, t_list* pokemons) {
    int menor_distacia = 10000;
    t_pokemon_mapeado* objetivo = NULL;

    void _procesar(void* pokemon) {
        char* especie = (char*) pokemon;
        t_list* ubicaciones = pokemon_get(especie);

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

    if (mi_objetivo == NULL || otro_objetivo == NULL) {
        log_error(default_logger, "Error al realizar intercambio: no se encontro alguno de los objetivos");
        exit(EXIT_FAILURE);
    }

    recibo->es_objetivo = entrego->es_objetivo = true;
    mi_objetivo->fue_capturado = otro_objetivo->fue_capturado = true;

    entrenador_verificar_objetivos(entrenador);
    entrenador_verificar_objetivos(otro_entrenador);

    log_debug(default_logger, "Se realizo un intercambio entre entrenador #%d (%s) y entrenador #%d (%s)",
            entrenador->id, entrego->nombre, otro_entrenador->id, recibo->nombre);
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

t_list* entrenador_get_bloqueados() {
    bool _is_blocked_full(void* elem) {
        t_entrenador* e = (t_entrenador*) elem;
        return e->estado == BLOCKED_FULL && !e->deadlock;
    }
    return list_filter(entrenadores, (void*)_is_blocked_full);
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
