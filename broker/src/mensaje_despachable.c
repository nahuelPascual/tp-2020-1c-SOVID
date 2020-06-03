/*
 * mensaje_despachable.c
 *
 *  Created on: 1 jun. 2020
 *      Author: utnso
 */

#include "mensaje_despachable.h"

t_mensaje_despachable* mensaje_despachable_crear(t_paquete* paquete) {
    t_mensaje_despachable* mensaje_despachable = malloc(sizeof(t_mensaje_despachable));
    mensaje_despachable->paquete = paquete;
    mensaje_despachable->suscriptores_a_los_que_fue_enviado = list_create();
    mensaje_despachable->suscriptores_que_lo_recibieron = list_create();

    pthread_mutex_init(&mensaje_despachable->mutex_ack, NULL);

    return mensaje_despachable;
}

void mensaje_despachable_liberar(t_mensaje_despachable* mensaje_despachable) {
    pthread_mutex_destroy(&mensaje_despachable->mutex_ack);
    list_destroy(mensaje_despachable->suscriptores_a_los_que_fue_enviado);
    list_destroy(mensaje_despachable->suscriptores_que_lo_recibieron);
    paquete_liberar(mensaje_despachable->paquete);
    free(mensaje_despachable);
}

bool mensaje_despachable_tiene_todos_los_acks(t_mensaje_despachable* mensaje_despachable) {
    int get_suscriptor(int suscriptor) {
        return suscriptor;
    }

    return list_equals_by(mensaje_despachable->suscriptores_a_los_que_fue_enviado,
                          mensaje_despachable->suscriptores_que_lo_recibieron,
                          (void*) get_suscriptor);
}

t_mensaje_despachable* mensaje_despachable_find_by_id_in(t_list* lista, uint32_t id_mensaje) {
    bool _is_the_one(t_mensaje_despachable* mensaje_despachable) {
        return mensaje_despachable->paquete->header->id_mensaje == id_mensaje;
    }

    return list_find(lista, (void*) _is_the_one);
}

void mensaje_despachable_remove_by_id_from(t_list* lista, uint32_t id_mensaje) {
    bool _is_the_one(t_mensaje_despachable* mensaje_despachable) {
        return mensaje_despachable->paquete->header->id_mensaje == id_mensaje;
    }

    list_remove_by_condition(lista, (void*) _is_the_one);
}
