//
// Created by utnso on 13/05/20.
//

#include "entrenador.h"

static t_list* entrenadores;
static t_dictionary* sem_entrenador; // array con los semaforos para sincronizar a los entrenadores

static int calcular_distancia(t_entrenador*, t_coord*);
static void avanzar(t_entrenador*);
static void log_entrenador(void*);
static void log_pokemon(void*);
static void log_objetivo(void*);
static bool entrenador_is_full(t_entrenador*);
static t_list* get_disponibles();
static t_coord* get_ubicacion_objetivo_actual(t_entrenador*);
static bool en_camino(t_entrenador*);
static void realizar_intercambio(t_entrenador*);
static sem_t* get_semaforo(int id);

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
        t_entrenador* e = (t_entrenador*) elem;
        dictionary_put(sem_entrenador, string_itoa(e->id), sem);
    }
    sem_entrenador = dictionary_create();
    list_iterate(entrenadores, _init_semaforos);
}

t_entrenador* entrenador_get_libre_mas_cercano(t_coord* posicion_buscada) {
    t_list* disponibles = get_disponibles();
    int menor = 10000, entrenador_mas_cercano;
    for (int i=0 ; i<list_size(disponibles) ; i++) {
        t_entrenador* entrenador = list_get(disponibles, i);
        int actual = calcular_distancia(entrenador, posicion_buscada);
        if (actual < menor) {
            menor = actual;
            entrenador_mas_cercano = entrenador->id;
        }
    }
    list_destroy(disponibles);
    return (t_entrenador*) list_get(entrenadores, entrenador_mas_cercano);
}

void entrenador_otorgar_ciclos_ejecucion(t_entrenador* entrenador, int cant) {
    for (int i = 0 ; i < cant ; i++) {
        sem_post(get_semaforo(entrenador->id));
    }
}

void entrenador_execute(t_entrenador* e) {
    while(1) {
        sem_t* sem = get_semaforo(e->id);
        if (sem_trywait(sem) != 0) { // el semaforo se bloquearia
            if (e->estado == EXECUTE || e->estado == BLOCKED_WAITING) {
                sem_post(&sem_post_ejecucion);
            }
            sem_wait(sem);
        }

        e->info->ciclos_cpu_ejecutados++;
        e->estado = EXECUTE;
        sleep(config_team->retardo_ciclo_cpu);

        if (en_camino(e)) {
            avanzar(e);
        } else if (e->deadlock) {
            if (--e->intercambio->remaining_intercambio < 1) {
                realizar_intercambio(e);
            }
        } else {
            enviar_catch_pokemon(e->id, e->pokemon_buscado);
            e->estado = BLOCKED_WAITING;
        }
    }
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
    return calcular_distancia(e, get_ubicacion_objetivo_actual(e)) + (e->deadlock ? 5 : 1);
}

static bool entrenador_is_full(t_entrenador* e) {
    return list_size(e->capturados) == list_size(e->objetivos);
}

void entrenador_verificar_objetivos(t_entrenador* e) {
    if (!entrenador_is_full(e)) {
        e->estado = BLOCKED_IDLE; // TODO deberia buscarle un pokemon del mapa? como saber cuales ya fueron asignados?
        return;
    }

    if (entrenador_cumplio_objetivos(e)) {
        e->estado = EXIT;
    } else {
        e->estado = BLOCKED_FULL;
    }
    sem_post(&sem_post_ejecucion);
}

bool entrenador_cumplio_objetivos(t_entrenador* e) {
    bool _cumple_objetivo(void* item) {
        t_pokemon_objetivo* obj = (t_pokemon_objetivo*) item;
        return obj->fue_capturado;
    }
    return list_all_satisfy(e->objetivos, (void*)_cumple_objetivo);
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
}

static t_list* get_disponibles() {
    bool _is_disponible(void* elemento) {
        t_entrenador* entrenador = (t_entrenador*) elemento;
        return entrenador->estado == NEW || entrenador->estado == BLOCKED_IDLE;
    }
    return list_filter(entrenadores, (void*)_is_disponible);
}

static int calcular_distancia(t_entrenador* e, t_coord* pos) {
    return (int) fabs((double)pos->x - (double)e->posicion->x) + fabs((double)pos->y - (double)e->posicion->y);
}

static t_coord* get_ubicacion_objetivo_actual(t_entrenador* e) {
    return e->deadlock ? e->intercambio->ubicacion : e->pokemon_buscado->ubicacion;
}

static bool en_camino(t_entrenador* e) {
    return calcular_distancia(e, get_ubicacion_objetivo_actual(e)) > 0;
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

static sem_t* get_semaforo(int id) {
    char* key = string_itoa(id);
    sem_t* sem = dictionary_get(sem_entrenador, key);
    free(key);
    return sem;
}
