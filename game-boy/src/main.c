/*
 * main.c
 *
 *  Created on: Apr 8, 2020
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/log.h>
#include <delibird-commons/utils/ipc.h>
#include <delibird-commons/utils/logger.h>
#include "mensaje.h"
#include "config.h"
#include "test/serialization.h"
#include "test/broker.h"

t_log* logger;
static int flag_test = 0;

int get_tipo_mensaje_from_args(char*);

int main(int argc, char **argv) {
    if(flag_test){
        iniciar_tests();
        return EXIT_SUCCESS;
    }

    char* proceso = argv[1];
    char* nombre_cola = argv[2];
    int tipo_mensaje = get_tipo_mensaje_from_args(nombre_cola);

    config_init("game-boy.config");
    char *ip, *puerto;

    logger = log_create("log_gameboy.txt", "GAMEBOY", true, LOG_LEVEL_INFO);

    t_paquete* paquete;
    uint32_t tiempo_suscripcion = 0;
    if(string_equals_ignore_case(proceso, "SUSCRIPTOR")) {
        tiempo_suscripcion = atoi(argv[3]);
        t_suscripcion* suscripcion = suscripcion_crear(tipo_mensaje, get_id_suscriptor(), tiempo_suscripcion);
        paquete = paquete_from_suscripcion(suscripcion);
        ip = get_ip("BROKER");
        puerto = get_puerto("BROKER");
        suscripcion_liberar(suscripcion);
    }
    else {
        paquete = resolver_mensaje(proceso, tipo_mensaje, argv);
        if (paquete == NULL) {
            log_error(logger, "ERROR DE CAPA 8");
            exit(EXIT_FAILURE);
        }
        string_to_upper(proceso);
        ip = get_ip(proceso);
        puerto = get_puerto(proceso);
    }

    int conexion = ipc_conectarse_a(ip, puerto);
    logger_conexion(logger, proceso, ip, puerto);
    ipc_enviar_a(conexion, paquete);
    logger_enviado(logger, paquete);

    if(string_equals_ignore_case(proceso, "SUSCRIPTOR")) {
        void _suscripcion() {
            log_info(logger, "Inicia suscripcion a cola %s", mensaje_get_tipo_as_string(tipo_mensaje));
            while(ipc_hay_datos_para_recibir_de(conexion)) {
                t_paquete* paquete = ipc_recibir_de(conexion);
                logger_recibido(logger, paquete);
                ipc_enviar_ack(get_id_suscriptor(), paquete->header->id_mensaje, conexion);
                paquete_liberar(paquete);
            }
        }

        pthread_t hilo;
        pthread_create(&hilo, NULL, (void*)_suscripcion, NULL);

        sleep(tiempo_suscripcion);
        log_info(logger, "Termina suscripcion a cola %s", mensaje_get_tipo_as_string(tipo_mensaje));

        pthread_cancel(hilo);
    }
    else {
        if(ipc_hay_datos_para_recibir_de(conexion)) {
            t_paquete* paquete_respuesta = ipc_recibir_de(conexion);
            logger_recibido(logger, paquete_respuesta);
            paquete_liberar(paquete_respuesta);
        }
    }

    paquete_liberar(paquete);
    ipc_cerrar(conexion);
    config_close();
    log_destroy(logger);

    return EXIT_SUCCESS;
}

int get_tipo_mensaje_from_args(char* tipoMensaje) {
    int cod_operacion = normalizar_tipo_mensaje(tipoMensaje);
    if (cod_operacion < 0) {
        log_error(logger, "%s no es un mensaje valido", tipoMensaje);
        exit(EXIT_FAILURE);
    }
    return cod_operacion;
}

