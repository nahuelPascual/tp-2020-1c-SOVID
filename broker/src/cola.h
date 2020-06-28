/*
 * cola.h
 *
 *  Created on: 17 may. 2020
 *      Author: utnso
 */

#ifndef TEST_COLA_H_
#define TEST_COLA_H_

#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>

#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>
#include <delibird-commons/utils/ipc.h>

#include "mensaje_despachable.h"

typedef struct {
    uint32_t id;
    int socket_asociado;
} t_suscriptor;

typedef struct {
    t_queue* mensajes_despachables;
    t_list* suscriptores;
    t_list* correlativos_recibidos;

    pthread_mutex_t mutex_mensajes_despachables;
    sem_t contador_mensajes_sin_despachar;
    pthread_mutex_t mutex_suscriptores;
    pthread_mutex_t mutex_correlativos_recibidos;
} t_cola;

t_cola* cola_crear();
t_dictionary* cola_crear_diccionario();

void cola_push_mensaje_despachable(t_cola* cola, t_mensaje_despachable* mensaje_despachable);
t_mensaje_despachable* cola_pop_mensaje_despachable(t_cola* cola);

void cola_add_or_update_suscriptor(t_cola* cola, uint32_t id_suscriptor, int socket_suscriptor);

void cola_despachar_mensaje_a_suscriptores(t_cola* cola, t_mensaje_despachable* mensaje_despachable, t_paquete* paquete);

#endif /* TEST_COLA_H_ */
