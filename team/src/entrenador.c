//
// Created by utnso on 13/05/20.
//

#include "entrenador.h"

static t_list* entrenadores;
static t_dictionary* sem_entrenador; // array con los semaforos para sincronizar a los entrenadores

static int calcular_remaining(t_entrenador* e, t_coord* pos);
static void avanzar(t_entrenador* e);
static void log_entrenador(void*);
static void log_pokemon(void*);
static void log_objetivo(void*);
static t_list* get_disponibles();

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
    entrenador->objetivo_actual = NULL;
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
        int actual = calcular_remaining(entrenador, posicion_buscada);
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
        sem_post((sem_t*)dictionary_get(sem_entrenador, string_itoa(entrenador->id)));
    }
}

void entrenador_execute(t_entrenador* e) {
    while(1) {
        char* id = string_itoa(e->id); // TODO sem_t* get_semaforo(id) para evitar leaks y simplificar codigo
        sem_t* sem = dictionary_get(sem_entrenador, id);
        free(id);
        if (sem_trywait(sem) != 0) { // el semaforo se bloquearia
            if (e->estado == EXECUTE) {
                sem_post(&sem_post_ejecucion);
            }
            sem_wait(sem);
        }

        e->info->ciclos_cpu_ejecutados++;
        e->estado = EXECUTE;
        sleep(config_team->retardo_ciclo_cpu);

        t_pokemon_mapeado* pokemon = e->objetivo_actual;
        if(calcular_remaining(e, pokemon->ubicacion) == 0) {
            enviar_catch_pokemon(pokemon);
            e->estado = BLOCKED_WAITING;
        } else {
            avanzar(e);
        }
    }
}

t_entrenador* entrenador_get(int id) {
    return list_get(entrenadores, id);
}

t_list* entrenador_get_execute() {
    bool _is_execute(void* elem) {
        t_entrenador* e = (t_entrenador*) elem;
        return e->estado == EXECUTE;
    }
    return list_filter(entrenadores, (void*)_is_execute);
}

t_list* entrenador_get_all() {
    return entrenadores;
}

int entrenador_get_count() {
    return list_size(entrenadores);
}

int entrenador_calcular_remaining(t_entrenador* e) {
    if (e->objetivo_actual == NULL) {
        log_error(default_logger, "Calculando remaining para entrenador #%d sin objetivo", e->id);
        return -1;
    }
    return calcular_remaining(e, e->objetivo_actual->ubicacion); // TODO calcular_distancia y +1(captura) +5(intercambio)
}

static void avanzar(t_entrenador* e) {
    t_coord* actual = e->posicion;
    t_coord* objetivo = e->objetivo_actual->ubicacion;
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

static int calcular_remaining(t_entrenador* e, t_coord* pos) {
    return (int) fabs((double)pos->x - (double)e->posicion->x) + fabs((double)pos->y - (double)e->posicion->y);
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
  list_iterate(entrenador->objetivos, (void*)log_pokemon);
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
