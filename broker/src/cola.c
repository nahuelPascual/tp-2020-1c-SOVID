/*
 * cola.c
 *
 *  Created on: 17 may. 2020
 *      Author: utnso
 */

#include "cola.h"

t_suscriptor* _crear_suscriptor(uint32_t id, int socket_asociado) {
    t_suscriptor* suscriptor = malloc(sizeof(t_suscriptor));
    suscriptor->id = id;
    suscriptor->socket_asociado = socket_asociado;

    return suscriptor;
}

t_cola* cola_crear() {
    t_cola* cola = malloc(sizeof(t_cola));
    cola->mensajes_despachables = queue_create();
    cola->suscriptores = list_create();
    cola->correlativos_recibidos = list_create();

    pthread_mutex_init(&cola->mutex_mensajes_despachables, NULL);
    sem_init(&cola->contador_mensajes_sin_despachar, 0, 0);
    pthread_mutex_init(&cola->mutex_suscriptores, NULL);
    pthread_mutex_init(&cola->mutex_correlativos_recibidos, NULL);

    return cola;
}

t_dictionary* cola_crear_diccionario() {
    t_dictionary* diccionario_de_colas = dictionary_create();

    dictionary_put(diccionario_de_colas, string_itoa(NEW_POKEMON), cola_crear());
    dictionary_put(diccionario_de_colas, string_itoa(APPEARED_POKEMON), cola_crear());
    dictionary_put(diccionario_de_colas, string_itoa(CATCH_POKEMON), cola_crear());
    dictionary_put(diccionario_de_colas, string_itoa(CAUGHT_POKEMON), cola_crear());
    dictionary_put(diccionario_de_colas, string_itoa(GET_POKEMON), cola_crear());
    dictionary_put(diccionario_de_colas, string_itoa(LOCALIZED_POKEMON), cola_crear());

    return diccionario_de_colas;
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

void cola_add_or_update_suscriptor(t_cola* cola, uint32_t id_suscriptor, int socket_suscriptor) {
    bool _is_the_one(t_suscriptor* suscriptor) {
        return suscriptor->id == id_suscriptor;
    }

    //TODO: Suscripciones por tiempo? Que pasa en caidas y recuperaciones?
    pthread_mutex_lock(&cola->mutex_suscriptores);
    t_suscriptor* suscriptor = list_find(cola->suscriptores, (void*) _is_the_one);

    if(suscriptor) {
        suscriptor->socket_asociado = socket_suscriptor;
    }
    else {
        list_add(cola->suscriptores, _crear_suscriptor(id_suscriptor, socket_suscriptor));
    }
    pthread_mutex_unlock(&cola->mutex_suscriptores);
}

void cola_despachar_mensaje_a_suscriptores(t_cola* cola, t_mensaje_despachable* mensaje_despachable) {
    void despachar_mensaje_a(t_suscriptor* suscriptor) {
        bool enviado = ipc_enviar_a(suscriptor->socket_asociado, mensaje_despachable->paquete);

        if(enviado) {
            list_add(mensaje_despachable->ids_suscriptores_a_los_que_fue_enviado, (void*) suscriptor->id);
        }
    }

    pthread_mutex_lock(&cola->mutex_suscriptores);
    list_iterate(cola->suscriptores, (void*) despachar_mensaje_a);
    pthread_mutex_unlock(&cola->mutex_suscriptores);
}
