/*
 * objetivos.c
 *
 *  Created on: 10 may. 2020
 *      Author: utnso
 */

#include "objetivos.h"

extern t_log* default_logger;

t_dictionary* mapa_objetivos;
t_dictionary* atrapados;

static void log_objetivos_globales(t_dictionary*);
static void actualizar_objetivos_globales(void* entrenador);

t_dictionary* calcular_objetivos_globales(t_list* entrenadores){
    mapa_objetivos = dictionary_create();
    atrapados = dictionary_create();
    list_iterate(entrenadores, &actualizar_objetivos_globales);
    sender_init_mensajes_esperando_respuesta();

    log_objetivos_globales(mapa_objetivos); //debug logging

    return mapa_objetivos;
}

static void actualizar_objetivos_globales(void* entrenador) {
    t_list* objetivos = ((t_entrenador*) entrenador)->objetivos;
    void _actualizar(void* elem){
      t_pokemon_objetivo* objetivo = (t_pokemon_objetivo*) elem;
      // solamente contamos en los objetivos globales aquellos pokemon pendientes de captura.
      // TODO ANALISIS: ver como garantizamos que no se capturen mas que los requeridos de cada especie (Issue: https://github.com/sisoputnfrba/foro/issues/1722#issuecomment-637854754)
      if(!objetivo->fue_capturado && dictionary_has_key(mapa_objetivos, objetivo->nombre)){
          int cant = (int) dictionary_get(mapa_objetivos, objetivo->nombre);
          dictionary_put(mapa_objetivos, objetivo->nombre, (void*) ++cant);
      } else {
          dictionary_put(mapa_objetivos, objetivo->nombre, (void*)1);
      }
    }
    list_iterate(objetivos, (void*)_actualizar);
}

t_list* objetivos_get_especies() {
    t_list* objetivos_globales = list_create();
    void _get_objetivo(char* k, void* v) {
        list_add(objetivos_globales, k);
    }
    dictionary_iterator(mapa_objetivos, (void*)_get_objetivo);
    return objetivos_globales;
}

bool is_pokemon_requerido(char* nombre) {
    if (!dictionary_has_key(mapa_objetivos, nombre)) {
        log_debug(default_logger, "%s no es un objetivo del team", nombre);
        return false;
    }
    int cantidad_objetivo = (int) dictionary_get(mapa_objetivos, nombre);

    if (!dictionary_has_key(atrapados, nombre)) {
        log_debug(default_logger, "Pokemon %s: capturados 0/%d", nombre, cantidad_objetivo);
        return true;
    }
    int cantidad_atrapado = (int) dictionary_get(atrapados, nombre);

    log_debug(default_logger, "Pokemon %s: capturados %d/%d", nombre, cantidad_atrapado, cantidad_objetivo);
    return cantidad_objetivo > cantidad_atrapado;
}

void objetivos_descontar_requeridos(char* pokemon) {
    int cant = (int) dictionary_get(mapa_objetivos, pokemon);
    if (cant) {
        dictionary_put(mapa_objetivos, pokemon, (void*) --cant);
        if (cant == 0) dictionary_remove(mapa_objetivos, pokemon);
    }
    else {
        log_warning(default_logger, "Se intento capturar mas %s que los requeridos por el team", pokemon);
    }
}

static void log_objetivos_globales(t_dictionary* mapa_objetivos){
    void _loggear(char* key, void* value){
         log_debug(default_logger, "        -Objetivo: %s (cant = %i)", key, (char*)value);
    }
    log_debug(default_logger, "Objetivos Globales: ");
    dictionary_iterator(mapa_objetivos, (void*)_loggear);
}

