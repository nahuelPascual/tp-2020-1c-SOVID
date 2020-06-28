/*
 * mensajeria.c
 *
 *  Created on: 17 may. 2020
 *      Author: utnso
 */

#include "mensajeria.h"

void _inicializar_buzon() {
    buzon = malloc(sizeof(t_buzon));
    buzon->diccionario_de_colas = cola_crear_diccionario();
    buzon->memoria = memoria_crear(0, 1024, FIRST_FIT);
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

//TODO: Separar en 2 funciones (bool _es_mensaje_redundante + void cola_add_correlativo_recibido)
bool _es_mensaje_redundante(t_paquete* paquete, t_cola* cola) {
    if(paquete_es_mensaje_de_respuesta(paquete)) {
        bool _condicion(uint32_t correlation_id_mensaje) {
            return paquete->header->correlation_id_mensaje == correlation_id_mensaje;
        }

        pthread_mutex_lock(&cola->mutex_correlativos_recibidos);
        bool fue_recibido = list_any_satisfy(cola->correlativos_recibidos, (void*) _condicion);

        if(!fue_recibido) {
            list_add(cola->correlativos_recibidos, (void*) paquete->header->correlation_id_mensaje);
        }
        pthread_mutex_unlock(&cola->mutex_correlativos_recibidos);

        return fue_recibido;
    }

    return false;
}

t_cola* mensajeria_get_cola(t_tipo_mensaje tipo_mensaje) {
    return dictionary_get(buzon->diccionario_de_colas, string_itoa(tipo_mensaje));
}

void _mensajeria_almacenar_paquete(t_paquete* paquete, t_cola* cola) {

    t_mensaje_despachable* mensaje_despachable = mensaje_despachable_from_paquete(paquete, buzon->memoria);

    memoria_dividir_particion_si_es_mas_grande_que(buzon->memoria, mensaje_despachable->particion_asociada, mensaje_despachable->size);

    memoria_asignar_paquete_a_la_particion(buzon->memoria, paquete, mensaje_despachable->particion_asociada);

    cola_push_mensaje_despachable(cola, mensaje_despachable);
}

void _procesar_paquete_de(t_paquete* paquete, int cliente) {
    switch(paquete->header->tipo_paquete) {
    case SUSCRIPCION: {
        t_suscripcion* suscripcion = paquete_to_suscripcion(paquete);

        t_cola* cola = mensajeria_get_cola(suscripcion->tipo_mensaje);

        cola_add_or_update_suscriptor(cola, suscripcion->id_suscriptor, cliente);

        //TODO: enviar los mensajes cacheados ???

        suscripcion_liberar(suscripcion);
        paquete_liberar(paquete);

        suscripcion_liberar(suscripcion);

        suscripcion_liberar(suscripcion);

        break;
    }
    case ACK: {
        t_ack* ack = paquete_to_ack(paquete);

        pthread_mutex_lock(&mutex_pendientes_de_ack);
        t_mensaje_despachable* mensaje_despachable = mensaje_despachable_find_by_id_in(mensajes_pendientes_de_ack, ack->id_mensaje);
        pthread_mutex_unlock(&mutex_pendientes_de_ack);

        pthread_mutex_lock(&mensaje_despachable->mutex_ack);
        mensaje_despachable_agregar_ack(mensaje_despachable, ack->id_suscriptor);
        bool mensaje_tiene_todos_los_acks = mensaje_despachable_tiene_todos_los_acks(mensaje_despachable);
        pthread_mutex_unlock(&mensaje_despachable->mutex_ack);

        if(mensaje_tiene_todos_los_acks) {
            pthread_mutex_lock(&mutex_pendientes_de_ack);
            mensaje_despachable_remove_by_id_from(mensajes_pendientes_de_ack, mensaje_despachable->id);
            pthread_mutex_unlock(&mutex_pendientes_de_ack);

            mensaje_despachable_liberar(mensaje_despachable);
        }

        ack_liberar(ack);

        break;
    }
    case MENSAJE: {
        t_cola* cola = mensajeria_get_cola(paquete->header->tipo_mensaje);

        if(!_es_mensaje_redundante(paquete, cola)) {
            pthread_mutex_lock(&mutex_id_mensaje);
            paquete->header->id_mensaje = id_mensaje++;
            pthread_mutex_unlock(&mutex_id_mensaje);

            _informar_id_mensaje_a(cliente, paquete->header->id_mensaje);

            _mensajeria_almacenar_paquete(paquete, cola);
        }

        break;
    }
    default:
        printf("Anda a saber que le llego al pobre broker");
        break;
    }

    paquete_liberar(paquete);
}

void _gestionar_a(int cliente) {
    while(ipc_hay_datos_para_recibir_de(cliente)) {
        t_paquete* paquete = ipc_recibir_de(cliente);
        _procesar_paquete_de(paquete, cliente);
    }
}

void mensajeria_inicializar() {
    _inicializar_buzon();
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
    t_cola* cola = mensajeria_get_cola(tipo_mensaje);

    while(1) {
        t_mensaje_despachable* mensaje_despachable = cola_pop_mensaje_despachable(cola);

        cola_despachar_mensaje_a_suscriptores(
                cola,
                mensaje_despachable,
                mensaje_despachable_to_paquete(mensaje_despachable, buzon->memoria));

        pthread_mutex_lock(&mutex_pendientes_de_ack);
        list_add(mensajes_pendientes_de_ack, mensaje_despachable);
        pthread_mutex_unlock(&mutex_pendientes_de_ack);
    }
}
