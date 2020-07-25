/*
 * cola.c
 *
 *  Created on: 17 may. 2020
 *      Author: utnso
 */

#include "cola.h"

t_cola* cola_crear(t_tipo_mensaje tipo_mensaje) {
    t_cola* cola = malloc(sizeof(t_cola));

    cola->tipo_mensaje = tipo_mensaje;

    cola->suscriptores = list_create();
    cola->mensajes_sin_despachar = queue_create();
    cola->mensajes_despachados = queue_create();

    pthread_mutex_init(&cola->mutex_suscriptores, NULL);
    pthread_mutex_init(&cola->mutex_mensajes_sin_despachar, NULL);
    pthread_mutex_init(&cola->mutex_mensajes_despachados, NULL);
    sem_init(&cola->contador_mensajes_sin_despachar, 0, 0);

    return cola;
}

void cola_push_mensaje_sin_despachar(t_cola* cola, t_mensaje_despachable* mensaje_despachable) {
    pthread_mutex_lock(&cola->mutex_mensajes_sin_despachar);
    queue_push(cola->mensajes_sin_despachar, mensaje_despachable);
    pthread_mutex_unlock(&cola->mutex_mensajes_sin_despachar);

    sem_post(&cola->contador_mensajes_sin_despachar);
}

t_mensaje_despachable* cola_pop_mensaje_sin_despachar(t_cola* cola) {
    sem_wait(&cola->contador_mensajes_sin_despachar);

    pthread_mutex_lock(&cola->mutex_mensajes_sin_despachar);
    t_mensaje_despachable* mensaje_despachable = queue_pop(cola->mensajes_sin_despachar);
    pthread_mutex_unlock(&cola->mutex_mensajes_sin_despachar);

    return mensaje_despachable;
}

void cola_push_mensaje_despachado(t_cola* cola, t_mensaje_despachable* mensaje_despachable) {
    pthread_mutex_lock(&cola->mutex_mensajes_despachados);
    queue_push(cola->mensajes_despachados, mensaje_despachable);
    pthread_mutex_unlock(&cola->mutex_mensajes_despachados);
}

void cola_iterate_mensajes_despachados(t_cola* cola, void (*closure)(t_mensaje_despachable*)) {
    pthread_mutex_lock(&cola->mutex_mensajes_despachados);
    list_iterate(cola->mensajes_despachados->elements, (void*) closure);
    pthread_mutex_unlock(&cola->mutex_mensajes_despachados);
}

void cola_add_or_update_suscriptor(t_cola* cola, t_suscriptor* un_suscriptor) {
    bool _is_the_one(t_suscriptor* suscriptor) {
        return suscriptor->id == un_suscriptor->id;
    }

    pthread_mutex_lock(&cola->mutex_suscriptores);
    list_remove_and_destroy_by_condition(cola->suscriptores, (void*) _is_the_one, (void*) suscriptor_liberar);
    list_add(cola->suscriptores, un_suscriptor);
    pthread_mutex_unlock(&cola->mutex_suscriptores);
}

void cola_iterate_suscriptores(t_cola* cola, void (*closure)(t_suscriptor*)) {
    pthread_mutex_lock(&cola->mutex_suscriptores);
    list_iterate(cola->suscriptores, (void*) closure);
    pthread_mutex_unlock(&cola->mutex_suscriptores);
}

bool cola_es_mensaje_redundante(t_cola* cola, t_paquete* paquete) {
    bool _is_the_one(t_mensaje_despachable* mensaje_despachable) {
        return paquete_es_mensaje_de_respuesta(paquete) && mensaje_despachable_es_misma_respuesta_que(mensaje_despachable, paquete);
    }

    pthread_mutex_lock(&cola->mutex_mensajes_sin_despachar);
    bool redundante = list_any_satisfy(cola->mensajes_sin_despachar->elements, (void*) _is_the_one);
    pthread_mutex_unlock(&cola->mutex_mensajes_sin_despachar);

    return redundante;
}

t_mensaje_despachable* cola_find_mensaje_despachable_by_id(t_cola* cola, uint32_t id) {
    bool _is_the_one(t_mensaje_despachable* mensaje_despachable) {
        return mensaje_despachable->id == id;
    }

    t_list* mensajes_despachables = list_create();

    pthread_mutex_lock(&cola->mutex_mensajes_sin_despachar);
    list_add_all(mensajes_despachables, cola->mensajes_sin_despachar->elements);
    pthread_mutex_unlock(&cola->mutex_mensajes_sin_despachar);

    pthread_mutex_lock(&cola->mutex_mensajes_despachados);
    list_add_all(mensajes_despachables, cola->mensajes_despachados->elements);
    pthread_mutex_unlock(&cola->mutex_mensajes_despachados);

    t_mensaje_despachable* mensaje_despachable = list_find(mensajes_despachables, (void*) _is_the_one);

    list_destroy(mensajes_despachables);

    return mensaje_despachable;
}

bool cola_remove_and_destroy_mensaje_despachable_by_id(t_cola* cola, uint32_t id) {
    bool _is_the_one(t_mensaje_despachable* mensaje_despachable) {
        return mensaje_despachable->id == id;
    }

    pthread_mutex_lock(&cola->mutex_mensajes_sin_despachar);
    t_mensaje_despachable* mensaje_sin_despachar = list_remove_by_condition(cola->mensajes_sin_despachar->elements, (void*) _is_the_one);
    pthread_mutex_unlock(&cola->mutex_mensajes_sin_despachar);

    if(mensaje_sin_despachar) {
        logger_mensaje_sin_despachar_eliminado(cola, mensaje_sin_despachar);

        sem_wait(&cola->contador_mensajes_sin_despachar);
        mensaje_despachable_liberar(mensaje_sin_despachar);
        return true;
    }

    pthread_mutex_lock(&cola->mutex_mensajes_despachados);
    t_mensaje_despachable* mensaje_despachado = list_remove_by_condition(cola->mensajes_despachados->elements, (void*) _is_the_one);
    pthread_mutex_unlock(&cola->mutex_mensajes_despachados);

    if(mensaje_despachado) {
        mensaje_despachable_liberar(mensaje_despachado);
        return true;
    }

    return false;
}
