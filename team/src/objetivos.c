/*
 * objetivos.c
 *
 *  Created on: 10 may. 2020
 *      Author: utnso
 */

#include "objetivos.h"

extern t_log* default_logger;

static t_dictionary* mapa_objetivos;
static t_dictionary* atrapados;

static void log_objetivos_globales();
static void actualizar_objetivos_globales(void* entrenador);
static void actualizar_capturados(void*);

t_dictionary* objetivos_globales_init(t_list* entrenadores) {
    mapa_objetivos = dictionary_create();
    list_iterate(entrenadores, &actualizar_objetivos_globales);

    atrapados = dictionary_create();
    list_iterate(entrenadores, actualizar_capturados);

    log_objetivos_globales(); //debug logging

    return mapa_objetivos;
}

static void actualizar_capturados(void* e) {
    t_list* capturados = ((t_entrenador*) e)->capturados;
    void _contar(void* c) {
        t_pokemon_capturado* p = (t_pokemon_capturado*) c;
        if(dictionary_has_key(atrapados, p->nombre)){
            int cant = (int) dictionary_get(atrapados, p->nombre);
            dictionary_put(atrapados, p->nombre, (void*) ++cant);
        } else {
            dictionary_put(atrapados, p->nombre, (void*)1);
        }
    }
    list_iterate(capturados, (void*)_contar);
}

static void actualizar_objetivos_globales(void* entrenador) {
    t_list* objetivos = ((t_entrenador*) entrenador)->objetivos;
    void _actualizar(void* elem){
        t_pokemon_objetivo* objetivo = (t_pokemon_objetivo*) elem;
        if(dictionary_has_key(mapa_objetivos, objetivo->nombre)){
            int cant = (int) dictionary_get(mapa_objetivos, objetivo->nombre);
            dictionary_put(mapa_objetivos, objetivo->nombre, (void*) ++cant);
        } else {
            dictionary_put(mapa_objetivos, objetivo->nombre, (void*)1);
        }
    }
    list_iterate(objetivos, (void*)_actualizar);
}

t_list* objetivos_get_especies_pendientes() {
    t_list* objetivos_globales = list_create();

    pthread_mutex_lock(&mx_atrapados);
    void _get_objetivo(char* k, void* v) {
        int objetivo = (int) v;
        int capturados = (int) dictionary_get(atrapados, k);
        if (capturados < objetivo) {
            list_add(objetivos_globales, k);
        }
    }
    dictionary_iterator(mapa_objetivos, (void*)_get_objetivo);
    pthread_mutex_unlock(&mx_atrapados);

    return objetivos_globales;
}

int objetivos_cantidad_pendientes(char* nombre) {
    int cantidad_objetivo = (int) dictionary_get(mapa_objetivos, nombre);
    if (cantidad_objetivo == 0) {
        log_debug(default_logger, "%s no es un objetivo del team", nombre);
        return 0;
    }

    int cantidad_atrapado = (int) dictionary_get(atrapados, nombre);

    log_debug(default_logger, "Pokemon %s: capturados %d/%d", nombre, cantidad_atrapado, cantidad_objetivo);
    return cantidad_objetivo - cantidad_atrapado;
}

void objetivos_capturado(char* pokemon) {
    pthread_mutex_lock(&mx_atrapados);
    int cant = (int) dictionary_get(atrapados, pokemon);
    dictionary_put(atrapados, pokemon, (void*) ++cant);
    pthread_mutex_unlock(&mx_atrapados);

    if (cant > (int)dictionary_get(mapa_objetivos, pokemon)) {
        log_error(default_logger, "Se capturaron mas %s que los requeridos por el team", pokemon);
    }
}

static void log_objetivos_globales(){
    void _loggear(char* key, void* value){
         log_debug(default_logger, "        -Objetivo: %s (cant = %i), Capturados %d", key, (char*)value, dictionary_get(atrapados, key));
    }
    log_debug(default_logger, "Objetivos Globales: ");
    dictionary_iterator(mapa_objetivos, (void*)_loggear);
}
