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
    if (string_equals_ignore_case(proceso, "SUSCRIPTOR")) {
        t_suscripcion* suscripcion = suscripcion_crear(tipo_mensaje, atoi(argv[3]));
        paquete = paquete_from_suscripcion(suscripcion);
        ip = get_ip("BROKER");
        puerto = get_puerto("BROKER");
        suscripcion_liberar(suscripcion);
    } else {
        paquete = resolver_mensaje(tipo_mensaje, argv);
        if (paquete == NULL) {
            log_error(logger, "El tipo de mensaje \"%s\" no es valido", nombre_cola);
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
        while (ipc_hay_datos_para_recibir_de(conexion)) {
            paquete = ipc_recibir_de(conexion);
            logger_recibido(logger, paquete);
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

