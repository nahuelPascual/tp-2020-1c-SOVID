/*
 * mensajeria.c
 *
 *  Created on: 17 may. 2020
 *      Author: utnso
 */

#include "mensajeria.h"

void _procesar_paquete_de(t_paquete* paquete, int cliente) {
    switch(paquete->header->tipo_paquete) {
    case SUSCRIPCION: {
        t_suscripcion* suscripcion = paquete_to_suscripcion(paquete);

        t_suscriptor* suscriptor = suscriptor_crear(suscripcion->id_suscriptor, suscripcion->tipo_mensaje, cliente);

        buzon_registrar_suscriptor(buzon, suscriptor);

        suscripcion_liberar(suscripcion);

        break;
    }
    case ACK: {
        t_ack* ack = paquete_to_ack(paquete);

        buzon_recibir_ack(buzon, ack);

        ack_liberar(ack);

        break;
    }
    case MENSAJE: {
        bool almacenado = buzon_almacenar_mensaje(buzon, paquete);

        if(almacenado)
            buzon_informar_id_mensaje_a(paquete->header->id_mensaje, cliente);

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

void _gestionar_clientes() {
    int broker = ipc_escuchar_en("127.0.0.1", "8081");

    while(1) {
        pthread_t gestor_de_un_cliente;
        int cliente = ipc_esperar_cliente(broker);
        pthread_create(&gestor_de_un_cliente, NULL, (void*) _gestionar_a, (void*) cliente);
        pthread_detach(gestor_de_un_cliente);
    }
}

void _despachar_mensajes_de(t_cola* cola) {
    while(1) {
        buzon_despachar_mensaje_de(buzon, cola);
    }
}

void mensajeria_inicializar() {
    buzon = buzon_crear();
}

void mensajeria_despachar_mensajes() {
    void _despachar_mensajes(char* key, t_cola* cola) {
        pthread_t gestor_de_una_cola;

        pthread_create(&gestor_de_una_cola, NULL, (void*) _despachar_mensajes_de, (void*) cola);
        pthread_detach(gestor_de_una_cola);
    }

    dictionary_iterator(buzon->administrador_colas->colas, (void*) _despachar_mensajes);
}

void mensajeria_gestionar_clientes() {
    pthread_t gestor_de_clientes;

    pthread_create(&gestor_de_clientes, NULL, (void*) _gestionar_clientes, NULL);
    pthread_join(gestor_de_clientes, NULL);
}
