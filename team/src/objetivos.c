/*
 * objetivos.c
 *
 *  Created on: 10 may. 2020
 *      Author: utnso
 */

#include "objetivos.h"

t_dictionary* mapa_objetivos;
t_dictionary* atrapados;

static void log_objetivos_globales(t_dictionary*);
static void log_objetivo_global(char*, char*);

t_dictionary* calcular_objetivos_globales(t_list* entrenadores){
    mapa_objetivos = dictionary_create();
    atrapados = dictionary_create();
    list_iterate(entrenadores, &calcular_objetivos_entrenador);

    log_objetivos_globales(mapa_objetivos); //debug logging

    return mapa_objetivos;
}

void calcular_objetivos_entrenador(void* entrenador){
    t_list* objetivos = ((t_entrenador*) entrenador)->objetivos;
    list_iterate(objetivos, &actualizar_objetivos_globales);
}

void actualizar_objetivos_globales(void* objetivo){
  if(dictionary_has_key(mapa_objetivos, objetivo)){
      int cant = (int) dictionary_get(mapa_objetivos, objetivo);
      dictionary_put(mapa_objetivos, objetivo, (void*) ++cant);
  }else{
      dictionary_put(mapa_objetivos, objetivo, (void*)1);
  }
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

static void log_objetivos_globales(t_dictionary* mapa_objetivos){
    log_debug(default_logger, "Objetivos Globales: ");
    dictionary_iterator(mapa_objetivos, &log_objetivo_global);
}

static void log_objetivo_global(char* key, char* value){
     log_debug(default_logger, "        -Objetivo: %s (cant = %i)", key, value);
}
