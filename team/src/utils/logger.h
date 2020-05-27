/*
 * logger.h
 *
 *  Created on: 10 may. 2020
 *      Author: utnso
 */

#ifndef UTILS_LOGGER_H_
#define UTILS_LOGGER_H_

#include <commons/log.h>
#include "parser.h"

extern char* const default_logger_path;
extern char* const config_path;

t_log* iniciar_default_logger();
t_log* iniciar_team_logger(char*);
void log_config_team(t_config_team*);
void log_pokemones(void*);
void log_entrenador(void*);
void log_objetivos(void*);
void log_objetivo_global(char*, void*);
void log_objetivos_globales(t_dictionary*);

#endif /* UTILS_LOGGER_H_ */
