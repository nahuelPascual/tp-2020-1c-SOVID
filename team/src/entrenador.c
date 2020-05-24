//
// Created by utnso on 13/05/20.
//

#include "entrenador.h"

t_list* entrenadores;
static pthread_mutex_t mx_execute;

static bool is_disponible(void* elemento);
static bool is_ready(void* elemento);
static void log_entrenador(void*);
static void log_pokemon(char*);

void entrenador_init_list(t_list* e) {
    entrenadores = list_create();
    list_add_all(entrenadores, e);
}

t_entrenador* entrenador_get_libre_mas_cercano(t_coord* posicion_buscada) {
    t_list* disponibles = list_filter(entrenadores, (void*)is_disponible);
    int menor = 10000, entrenador_mas_cercano;
    for (int i=0 ; i<list_size(disponibles) ; i++) {
        t_entrenador* entrenador = (t_entrenador*) list_get(disponibles, i);
        int actual = (int) fabs((double)posicion_buscada->x - (double)entrenador->posicion->x);
        actual += (int)fabs((double)posicion_buscada->y - (double)entrenador->posicion->y);
        if (actual < menor) {
            menor = actual;
            entrenador_mas_cercano = entrenador->thread_id;
        }
    }

    return (t_entrenador*) list_get(entrenadores, entrenador_mas_cercano);
}

t_list* entrenador_filtrar_ready() {
    return list_filter(entrenadores,(void*)is_ready);
}

void entrenador_execute(t_entrenador* e) {
    printf("mutex\n");
    printf("ejecutando entrenador %d", e->thread_id);
}

t_list* entrenador_all() {
    return entrenadores;
}

void entrenador_set_ready(t_entrenador* e) {
    e->estado = READY;
    queue_push(cola_ready, e);
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
  list_iterate(entrenador->pokemon_atrapados, &log_pokemon);
}

static void log_pokemon(char* pokemon){
    log_debug(default_logger, "                         -%s", pokemon);
}
