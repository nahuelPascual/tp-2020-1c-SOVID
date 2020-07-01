/*
 * cola.h
 *
 *  Created on: 17 may. 2020
 *      Author: utnso
 */

#ifndef COLA_H_
#define COLA_H_

#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>

#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>
#include <delibird-commons/utils/ipc.h>

#include "mensaje_despachable.h"

typedef struct {
    t_list* suscriptores;
    t_queue* mensajes_sin_despachar;
    t_queue* mensajes_despachados;

    pthread_mutex_t mutex_suscriptores;
    pthread_mutex_t mutex_mensajes_sin_despachar;
    pthread_mutex_t mutex_mensajes_despachados;
    sem_t contador_mensajes_sin_despachar;
} t_cola;

t_cola* cola_crear();
t_dictionary* cola_crear_diccionario();

void cola_push_mensaje_sin_despachar(t_cola* cola, t_mensaje_despachable* mensaje_despachable);
t_mensaje_despachable* cola_pop_mensaje_sin_despachar(t_cola* cola);
void cola_push_mensaje_despachado(t_cola* cola, t_mensaje_despachable* mensaje_despachable);
void cola_iterate_mensajes_despachados(t_cola* cola, void (*closure)(t_mensaje_despachable*));

void cola_add_or_update_suscriptor(t_cola* cola, t_suscriptor* suscriptor);
void cola_iterate_suscriptores(t_cola* cola, void (*closure)(t_suscriptor*));

bool cola_es_mensaje_redundante(t_cola* cola, t_paquete* paquete);

t_mensaje_despachable* cola_find_mensaje_despachable_by_id(t_cola* cola, uint32_t id);

#endif /* COLA_H_ */
