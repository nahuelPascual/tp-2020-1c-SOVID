/*
 * administrador_colas.c
 *
 *  Created on: 17 jun. 2020
 *      Author: utnso
 */

#include "administrador_colas.h"

t_administrador_colas* administrador_colas_crear() {
    t_administrador_colas* administrador_colas = malloc(sizeof(t_administrador_colas));
    administrador_colas->id_mensaje = 1;
    administrador_colas->colas = dictionary_create();

    for(t_tipo_mensaje tipo_mensaje = 0; tipo_mensaje < 6; tipo_mensaje++) {
        char* key = string_itoa(tipo_mensaje);
        dictionary_put(administrador_colas->colas, key, cola_crear(tipo_mensaje));
        free(key);
    }

    return administrador_colas;
}

t_cola* administrador_colas_get_cola_from(t_administrador_colas* administrador_colas, t_tipo_mensaje tipo_mensaje) {
    char* key = string_itoa(tipo_mensaje);
    t_cola* cola = dictionary_get(administrador_colas->colas, key);
    free(key);

    return cola;
}

void administrador_colas_asignar_id_mensaje_a(t_administrador_colas* administrador_colas, t_mensaje_despachable* mensaje_despachable) {
    uint32_t id_mensaje = administrador_colas->id_mensaje++;
    mensaje_despachable->id = id_mensaje;
    mensaje_despachable->particion_asociada->id_mensaje_asociado = id_mensaje;
}

t_mensaje_despachable* administrador_colas_find_mensaje_despachable_by_id(t_administrador_colas* administrador_colas, uint32_t id_mensaje) {
    t_mensaje_despachable* mensaje_despachable = NULL;

    for(t_tipo_mensaje tipo_mensaje = 0; !mensaje_despachable && tipo_mensaje < dictionary_size(administrador_colas->colas); tipo_mensaje++) {
        t_cola* cola = administrador_colas_get_cola_from(administrador_colas, tipo_mensaje);

        mensaje_despachable = cola_find_mensaje_despachable_by_id(cola, id_mensaje);
    }

    return mensaje_despachable;
}

bool administrador_colas_remove_and_destroy_mensaje_despachable_by_id(t_administrador_colas* administrador_colas, uint32_t id_mensaje) {
    bool eliminado = false;

    for(t_tipo_mensaje tipo_mensaje = 0; !eliminado && tipo_mensaje < dictionary_size(administrador_colas->colas); tipo_mensaje++) {
        t_cola* cola = administrador_colas_get_cola_from(administrador_colas, tipo_mensaje);

        eliminado = cola_remove_and_destroy_mensaje_despachable_by_id(cola, id_mensaje);
    }

    return eliminado;
}

t_cola* administrador_colas_find_cola_by_id_mensaje(t_administrador_colas* administrador_colas, uint32_t id_mensaje) {
    for(t_tipo_mensaje tipo_mensaje = 0; tipo_mensaje < dictionary_size(administrador_colas->colas); tipo_mensaje++) {
        t_cola* cola = administrador_colas_get_cola_from(administrador_colas, tipo_mensaje);

        if(cola_find_mensaje_despachable_by_id(cola, id_mensaje)) {
            return cola;
        }
    }

    return NULL;
}
