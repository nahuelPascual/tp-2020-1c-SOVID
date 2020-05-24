/*
 * parser.h
 *
 *  Created on: 10 may. 2020
 *      Author: utnso
 */

#ifndef UTILS_PARSER_H_
#define UTILS_PARSER_H_

#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include "../entrenador.h"

typedef struct {
    int tiempo_reconexion;
    int retardo_ciclo_cpu;
    char* algoritmo_planificacion;
    int quantum;
    int estimacion_inicial;
    char* ip_broker;
    char* puerto_broker;
    char* log_file;
    t_config* config;
} t_config_team;

t_log* logger;
t_log* default_logger;

t_config_team* parser_get_config_team();
void liberar_config_team(t_config_team*);

#endif
