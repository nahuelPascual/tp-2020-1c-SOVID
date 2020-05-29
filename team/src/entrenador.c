//
// Created by utnso on 13/05/20.
//

#include "entrenador.h"

static t_list* entrenadores;
static t_dictionary* sem_entrenador; // array con los semaforos para sincronizar a los entrenadores
static pthread_mutex_t mx_execute;

static bool is_disponible(void* elemento);
static bool is_ready(void* elemento);
static int calcular_remaining(t_entrenador* e, t_coord* pos);
static void avanzar(t_entrenador* e);
static void log_entrenador(void*);
static void log_pokemon(char*);

static t_info* metricas_new(){
    t_info* metricas = malloc(sizeof(t_info));

    metricas->cpu = 0;
    metricas->rafaga_actual = 0;
    metricas->cant_rafagas = 0;

    return metricas;
}

t_entrenador* entrenador_new(int id, t_list* objetivos, t_list* capturados, t_coord* posicion){
    t_entrenador* entrenador = malloc(sizeof(t_entrenador));

    entrenador->id = id;
    entrenador->objetivos = objetivos;
    entrenador->capturados = capturados;
    entrenador->posicion = posicion;
    entrenador->estado = NEW;
    entrenador->objetivo_actual = NULL;
    entrenador->metricas = metricas_new();

    return entrenador;
}

void entrenador_init_list(t_list* e) {
    pthread_mutex_init(&mx_execute, NULL);
    entrenadores = list_create();
    list_add_all(entrenadores, e);
    void _init_semaforos(void* e) {
        sem_t newSem;
        sem_init(&newSem, 0, 0);
        sem_t* sem = malloc(sizeof(sem_t));
        memcpy(sem, &newSem, sizeof(sem_t));
        dictionary_put(sem_entrenador, string_itoa(((t_entrenador*)e)->id), sem);
    }
    sem_entrenador = dictionary_create();
    list_iterate(entrenadores, _init_semaforos);
}

t_entrenador* entrenador_get_libre_mas_cercano(t_coord* posicion_buscada) {
    t_list* disponibles = list_filter(entrenadores, (void*)is_disponible);
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

t_list* entrenador_filtrar_ready() {
    return list_filter(entrenadores,(void*)is_ready);
}

void entrenador_otorgar_ciclos_ejecucion(t_entrenador* entrenador, int cant) {
    for (int i = 0 ; i < cant ; i++) {
        sem_post((sem_t*)dictionary_get(sem_entrenador, entrenador->id));
    }
}

void entrenador_execute(t_entrenador* e) {
    while(1) {
        sem_wait(dictionary_get(sem_entrenador, string_itoa(e->id)));
        pthread_mutex_lock(&mx_execute);

        e->estado = EXECUTE;
        sleep(config_team->retardo_ciclo_cpu);

        t_pokemon* pokemon = e->objetivo_actual;
        if(calcular_remaining(e, pokemon->ubicacion) == 0) {
            enviar_catch_pokemon(pokemon);
            e->estado = BLOCKED_WAITING;
        } else {
            avanzar(e);
        }

        pthread_mutex_unlock(&mx_execute);
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

int entrenador_calcular_remaining(t_entrenador* e, t_coord* pos) {
    if (e->objetivo_actual == NULL) {
        return 0;
    }
    return calcular_remaining(e, pos);
}

void entrenador_set_ready(t_entrenador* e) {
    e->estado = READY;
    queue_push(cola_ready, e);
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

static int calcular_remaining(t_entrenador* e, t_coord* pos) {
    return (int) fabs((double)pos->x - (double)e->posicion->x) + fabs((double)pos->y - (double)e->posicion->y);
}

static bool is_disponible(void* elemento) {
    t_entrenador* entrenador = (t_entrenador*) elemento;
    return entrenador->estado == NEW || entrenador->estado == BLOCKED_IDLE;
}

static bool is_ready(void* elemento) {
    t_entrenador* entrenador = (t_entrenador*) elemento;
    return entrenador->estado == READY;
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
  list_iterate(entrenador->objetivos, &log_pokemon);
  log_debug(default_logger, "               *Atrapados: ");
  list_iterate(entrenador->capturados, &log_pokemon);
}

static void log_pokemon(char* pokemon){
    log_debug(default_logger, "                         -%s", pokemon);
}
