/*
 * cola.c
 *
 *  Created on: 17 may. 2020
 *      Author: utnso
 */

#include "cola.h"

t_cola* cola_crear() {
    t_cola* cola = malloc(sizeof(t_cola));
    cola->mensajes_despachables = queue_create();
    cola->suscriptores = list_create();
    cola->correlativos_recibidos = list_create();

    pthread_mutex_init(&cola->mutex_mensajes_despachables, NULL);
    sem_init(&cola->contador_mensajes_sin_despachar, 0, 0);
    pthread_mutex_init(&cola->mutex_suscriptores, NULL);
    pthread_mutex_init(&cola->mutex_correlativos, NULL);

    return cola;
}

void cola_push_mensaje_despachable(t_cola* cola, t_mensaje_despachable* mensaje_despachable) {
    pthread_mutex_lock(&cola->mutex_mensajes_despachables);
    queue_push(cola->mensajes_despachables, mensaje_despachable);
    pthread_mutex_unlock(&cola->mutex_mensajes_despachables);

    sem_post(&cola->contador_mensajes_sin_despachar);
}

t_mensaje_despachable* cola_pop_mensaje_despachable(t_cola* cola) {
    sem_wait(&cola->contador_mensajes_sin_despachar);

    pthread_mutex_lock(&cola->mutex_mensajes_despachables);
    t_mensaje_despachable* mensaje_despachable = queue_pop(cola->mensajes_despachables);
    pthread_mutex_unlock(&cola->mutex_mensajes_despachables);

    return mensaje_despachable;
}

void cola_add_suscriptor(t_cola* cola, int suscriptor) {
    pthread_mutex_lock(&cola->mutex_suscriptores);
    list_add(cola->suscriptores, (void*) suscriptor); //TODO: suscripcion->tiempo PARA QUE LO USO?
    pthread_mutex_unlock(&cola->mutex_suscriptores);
}

void cola_despachar_mensaje_a_suscriptores(t_cola* cola, t_mensaje_despachable* mensaje_despachable) {
    void despachar_mensaje_a(int suscriptor) {
        ipc_enviar_a(suscriptor, mensaje_despachable->paquete); //TODO: bool seEnvio = ipc_enviar...
        list_add(mensaje_despachable->suscriptores_a_los_que_fue_enviado, (void*) suscriptor);
    }

    pthread_mutex_lock(&cola->mutex_suscriptores);
    list_iterate(cola->suscriptores, (void*) despachar_mensaje_a);
    pthread_mutex_unlock(&cola->mutex_suscriptores);
}
