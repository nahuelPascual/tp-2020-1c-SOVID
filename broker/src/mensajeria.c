/*
 * mensajeria.c
 *
 *  Created on: 17 may. 2020
 *      Author: utnso
 */

#include "mensajeria.h"

void _inicializar_colas() {
    diccionario_de_colas = dictionary_create();
    dictionary_put(diccionario_de_colas, string_itoa(NEW_POKEMON), cola_crear());
    dictionary_put(diccionario_de_colas, string_itoa(APPEARED_POKEMON), cola_crear());
    dictionary_put(diccionario_de_colas, string_itoa(CATCH_POKEMON), cola_crear());
    dictionary_put(diccionario_de_colas, string_itoa(CAUGHT_POKEMON), cola_crear());
    dictionary_put(diccionario_de_colas, string_itoa(GET_POKEMON), cola_crear());
    dictionary_put(diccionario_de_colas, string_itoa(LOCALIZED_POKEMON), cola_crear());
}

void _inicializar_semaforos() {
    pthread_mutex_init(&mutex_pendientes_de_ack, NULL);
    pthread_mutex_init(&mutex_id_mensaje, NULL);
}

void _informar_id_mensaje_a(int cliente, int id_mensaje) {
    t_informe_id* informe_id = informe_id_crear(id_mensaje);
    t_paquete* paquete = paquete_from_informe_id(informe_id);
    ipc_enviar_a(cliente, paquete);
    paquete_liberar(paquete);
    informe_id_liberar(informe_id);
}

void _procesar_paquete_de(t_paquete* paquete, int cliente) {
    switch(paquete->header->tipo_paquete) {
    case SUSCRIPCION: {
        t_suscripcion* suscripcion = paquete_to_suscripcion(paquete);

        t_cola* cola = dictionary_get(diccionario_de_colas, string_itoa(suscripcion->tipo_mensaje));

        cola_add_suscriptor(cola, cliente);

        break;
    }
    case ACK: {
        t_ack* ack = paquete_to_ack(paquete);

        pthread_mutex_lock(&mutex_pendientes_de_ack);
        t_mensaje_despachable* mensaje_despachable = mensaje_despachable_find_by_id_in(mensajes_pendientes_de_ack, ack->id_mensaje);
        pthread_mutex_unlock(&mutex_pendientes_de_ack);

        pthread_mutex_lock(&mensaje_despachable->mutex_ack);
        list_add(mensaje_despachable->suscriptores_que_lo_recibieron, (void*) cliente);
        bool mensaje_tiene_todos_los_acks = mensaje_despachable_tiene_todos_los_acks(mensaje_despachable);
        pthread_mutex_lock(&mensaje_despachable->mutex_ack);

        if(mensaje_tiene_todos_los_acks) {
            pthread_mutex_lock(&mutex_pendientes_de_ack);
            mensaje_despachable_remove_by_id_from(mensajes_pendientes_de_ack, mensaje_despachable->paquete->header->id_mensaje);
            pthread_mutex_unlock(&mutex_pendientes_de_ack);

            mensaje_despachable_liberar(mensaje_despachable);
        }

        break;
    }
    case MENSAJE: {
        pthread_mutex_lock(&mutex_id_mensaje);
        paquete->header->id_mensaje = id_mensaje++;
        pthread_mutex_unlock(&mutex_id_mensaje);

        _informar_id_mensaje_a(cliente, paquete->header->id_mensaje);

        t_cola* cola = dictionary_get(diccionario_de_colas, string_itoa(paquete->header->tipo_mensaje));

        t_mensaje_despachable* mensaje_despachable = mensaje_despachable_crear(paquete);

        cola_push_mensaje_despachable(cola, mensaje_despachable);

        break;
    }
    default:
        printf("Anda a saber que le llego al pobre broker");
        break;
    }
}

void _gestionar_a(int cliente) {
    while(ipc_hay_datos_para_recibir_de(cliente)) {
        t_paquete* paquete = ipc_recibir_de(cliente);
        _procesar_paquete_de(paquete, cliente);
    }
}

void mensajeria_inicializar() {
    _inicializar_colas();
    _inicializar_semaforos();
    mensajes_pendientes_de_ack = list_create();
    id_mensaje = 1;
}

void mensajeria_gestionar_clientes() {
    int broker = ipc_escuchar_en("127.0.0.1", "8081");

    while(1) {
        pthread_t gestor_de_un_cliente;
        int cliente = ipc_esperar_cliente(broker);
        pthread_create(&gestor_de_un_cliente, NULL, (void*) _gestionar_a, (void*) cliente);
        pthread_detach(gestor_de_un_cliente);
    }
}

void mensajeria_despachar_mensajes_de(t_tipo_mensaje tipo_mensaje) {
    t_cola* cola = dictionary_get(diccionario_de_colas, string_itoa(tipo_mensaje));

    while(1) {
        t_mensaje_despachable* mensaje_despachable = cola_pop_mensaje_despachable(cola);

        cola_despachar_mensaje_a_suscriptores(cola, mensaje_despachable);

        pthread_mutex_lock(&mutex_pendientes_de_ack);
        list_add(mensajes_pendientes_de_ack, mensaje_despachable);
        pthread_mutex_unlock(&mutex_pendientes_de_ack);
    }
}
