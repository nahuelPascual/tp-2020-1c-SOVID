/*
 * configuracion.c
 *
 *  Created on: 14 jul. 2020
 *      Author: utnso
 */

#include "configuracion.h"

static char* const config_path = "broker.config";

t_configuracion* configuracion_crear() {
    t_configuracion* configuracion = malloc(sizeof(t_configuracion));
    t_config* config = config_create(config_path);
    bool configuracion_valida = true;

    if(!config) {
        logger_archivo_no_encontrado(config_path);
        exit(EXIT_FAILURE);
    }

    configuracion->tamanio_memoria = config_get_int_value(config, "TAMANO_MEMORIA");
    configuracion->tamanio_minimo_particion = config_get_int_value(config, "TAMANO_MINIMO_PARTICION");

    char* algoritmo_memoria = config_get_string_value(config, "ALGORITMO_MEMORIA");
    if(string_equals_ignore_case(algoritmo_memoria, "PARTICIONES")) {
        configuracion->algoritmo_memoria = PARTICIONES_DINAMICAS;
    }
    else if(string_equals_ignore_case(algoritmo_memoria, "BS")) {
        configuracion->algoritmo_memoria = BUDDY_SYSTEM;
    }
    else {
        configuracion_valida = false;
    }

    char* algoritmo_reemplazo = config_get_string_value(config, "ALGORITMO_REEMPLAZO");
    if(string_equals_ignore_case(algoritmo_reemplazo, "FIFO")) {
        configuracion->algoritmo_reemplazo = FIFO;
    }
    else if(string_equals_ignore_case(algoritmo_reemplazo, "LRU")) {
        configuracion->algoritmo_reemplazo = LRU;
    }
    else {
        configuracion_valida = false;
    }

    char* algoritmo_particion_libre = config_get_string_value(config, "ALGORITMO_PARTICION_LIBRE");
    if(string_equals_ignore_case(algoritmo_particion_libre, "FF")) {
        configuracion->algoritmo_particion_libre = FIRST_FIT;
    }
    else if(string_equals_ignore_case(algoritmo_particion_libre, "BF")) {
        configuracion->algoritmo_particion_libre = BEST_FIT;
    }
    else {
        configuracion_valida = false;
    }

    configuracion->ip_broker = string_duplicate(config_get_string_value(config, "IP_BROKER"));
    configuracion->puerto_broker = string_duplicate(config_get_string_value(config, "PUERTO_BROKER"));
    configuracion->frecuencia_compactacion = config_get_int_value(config, "FRECUENCIA_COMPACTACION");
    configuracion->log_file = string_duplicate(config_get_string_value(config, "LOG_FILE"));
    configuracion->dump_file = string_duplicate(config_get_string_value(config, "DUMP_FILE"));

    if(!configuracion_valida) {
        logger_archivo_invalido(config_path);
        exit(EXIT_FAILURE);
    }

    config_destroy(config);

    return configuracion;
}
