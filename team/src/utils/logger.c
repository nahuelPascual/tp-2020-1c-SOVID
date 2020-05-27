/*
 * logger.c
 *
 *  Created on: 10 may. 2020
 *      Author: utnso
 */

#include "logger.h"
#include "parser.h"

char* const default_logger_path = "default.log";
char* const config_path = "team.config";


t_log* iniciar_default_logger() {
    return log_create(default_logger_path, "TEAM", true, LOG_LEVEL_DEBUG);
}

t_log* iniciar_team_logger(char* logger_path) {
    return log_create(logger_path, "TEAM", true, LOG_LEVEL_DEBUG);
}

void log_config_team(t_config_team* config_team){
    log_debug(logger, string_from_format("Tiempo Reconexion: %i", config_team->tiempo_reconexion));
    log_debug(logger, string_from_format("Retardo Ciclo CPU: %i", config_team->retardo_ciclo_cpu));
    log_debug(logger, string_from_format("Algoritmo Planificacion: %s", config_team->algoritmo_planificacion));
    log_debug(logger, string_from_format("Quantum: %i", config_team->quantum));
    log_debug(logger, string_from_format("Estimacion Inicial: %i", config_team->estimacion_inicial));
    log_debug(logger, string_from_format("IP Broker: %s", config_team->ip_broker));
    log_debug(logger, string_from_format("IP Puerto: %s", config_team->puerto_broker));
    log_debug(logger, string_from_format("Log File: %s", config_team->log_file));
}


void log_entrenador(void* element){
  t_entrenador* entrenador = element;
  log_debug(logger, "       Entrenador:");
  log_debug(logger, "               *Posicision: x=%i ; y=%i", entrenador->posicion->x, entrenador->posicion->y);
  log_debug(logger, "               *Objetivos: ");
  list_iterate(entrenador->objetivos, &log_objetivos);
  log_debug(logger, "               *Pokemones: ");
  list_iterate(entrenador->capturados, &log_pokemones);
}

void log_objetivos(void* element){
    char* pokemon = element;
    log_debug(logger, "                         -%s", pokemon);
}

void log_pokemones(void* element){
    char* pokemon = element;
    log_debug(logger, "                         -%s", pokemon);
}

void log_objetivos_globales(t_dictionary* mapa_objetivos){
    log_debug(logger, "Objetivos Globales: ");
    dictionary_iterator(mapa_objetivos, &log_objetivo_global);
}

void log_objetivo_global(char* key, void* value){
     log_debug(logger, "        -Objetivo: %s (cant = %i)", key, value);
}
