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

        logger_suscripcion_recibida(suscripcion);

        suscripcion_liberar(suscripcion);
        break;
    }
    case ACK: {
        t_ack* ack = paquete_to_ack(paquete);

        logger_ack_recibido(ack);

        buzon_recibir_ack(buzon, ack);

        ack_liberar(ack);
        break;
    }
    case MENSAJE: {
        logger_mensaje_recibido(paquete);

        t_mensaje_despachable* mensaje_despachable = buzon_almacenar_mensaje(buzon, paquete);

        mensaje_despachable_informar_id_a(mensaje_despachable, cliente);
        break;
    }
    default:
        logger_anda_a_saber_que_llego();
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
    int broker = ipc_escuchar_en(configuracion->ip_broker, configuracion->puerto_broker);

    while(1) {
        int cliente = ipc_esperar_cliente(broker);

        logger_conexion_iniciada(cliente);

        pthread_t gestor_de_un_cliente;
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
    logger_debug = log_create("broker_debug.log", "BROKER", true, LOG_LEVEL_DEBUG);
    configuracion = configuracion_crear();
    logger = log_create(configuracion->log_file, "BROKER", true, LOG_LEVEL_INFO);
    buzon = buzon_crear(
        configuracion->tamanio_memoria,
        configuracion->tamanio_minimo_particion,
        configuracion->algoritmo_memoria,
        configuracion->algoritmo_particion_libre,
        configuracion->algoritmo_reemplazo,
        configuracion->frecuencia_compactacion
    );

    logger_iniciando_broker(configuracion->tamanio_memoria);
}

void mensajeria_gestionar_signal(int signal) {
    if(signal == SIGUSR1) {
        buzon_imprimir_estado_en(buzon, configuracion->dump_file);

        logger_dump_ejecutado(configuracion->dump_file);
    }
}

void mensajeria_despachar_mensajes() {
    void _despachar_mensajes(char* key, t_cola* cola) {
        pthread_t gestor_de_una_cola;
        pthread_create(&gestor_de_una_cola, NULL, (void*) _despachar_mensajes_de, (void*) cola);
        pthread_detach(gestor_de_una_cola);

        logger_iniciando_despacho_de_mensajes_de(cola->tipo_mensaje);
    }

    dictionary_iterator(buzon->administrador_colas->colas, (void*) _despachar_mensajes);
}

void mensajeria_gestionar_clientes() {
    pthread_t gestor_de_clientes;
    pthread_create(&gestor_de_clientes, NULL, (void*) _gestionar_clientes, NULL);

    logger_iniciando_escucha_de_clientes();

    pthread_join(gestor_de_clientes, NULL);
}
