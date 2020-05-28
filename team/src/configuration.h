/*
 * configuration.h
 *
 *  Created on: 10 may. 2020
 *      Author: utnso
 */

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <delibird-commons/model/mensaje.h>
#include "entrenador.h"

typedef struct {
    int tiempo_reconexion;
    int retardo_ciclo_cpu;
    char* algoritmo_planificacion;
    int quantum;
    int estimacion_inicial;
    char* ip_broker;
    char* puerto_broker;
    char* ip_team;
    char* puerto_team;
    char* log_file;
} t_config_team;

t_log* logger;
t_log* default_logger;
t_config_team* config_team;

void config_team_init();
void liberar_config_team();

#endif
