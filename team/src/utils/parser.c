/*
 * parser.c
 *
 *  Created on: 10 may. 2020
 *      Author: utnso
 */

#include "parser.h"

static char* const default_logger_path = "default.log";
static char* const config_path = "team.config";
static t_config* config;

static t_log* iniciar_default_logger();
static void leer_config();
static t_list* parse_entrenadores();
static t_coord* parse_posicion_entrenador(char*);
static t_list* parse_pokemones(char*);
static t_log* iniciar_team_logger(char*);
static void log_config_team(t_config_team*);

void config_team_init() {
    default_logger = iniciar_default_logger();

    config_team = malloc(sizeof(t_config_team));
    leer_config();

    t_list* entrenadores = parse_entrenadores();
    log_entrenadores(entrenadores); // logeo los entrenadores
    entrenador_init_list(entrenadores);
    list_destroy(entrenadores);

    config_team->tiempo_reconexion = config_get_int_value(config, "TIEMPO_RECONEXION");
    config_team->retardo_ciclo_cpu = config_get_int_value(config, "RETARDO_CICLO_CPU");
    config_team->algoritmo_planificacion = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
    config_team->quantum = config_get_int_value(config, "QUANTUM");
    config_team->estimacion_inicial = config_get_int_value(config, "ESTIMACION_INICIAL");
    config_team->ip_broker = config_get_string_value(config, "IP_BROKER");
    config_team->puerto_broker = config_get_string_value(config, "PUERTO_BROKER");
    config_team->log_file = config_get_string_value(config, "LOG_FILE");

    logger = iniciar_team_logger(config_team->log_file);

    log_config_team(config_team); // logeo la config
}

static void leer_config() {
    config = config_create(config_path);
    if (config == NULL) {
        log_info(default_logger, "Archivo de configuracion no encontrado: %s", config_path);
        exit(EXIT_FAILURE);
    }
}

static t_list* parse_entrenadores(){
   t_list* entrenadores = list_create();

   char** array_posiciones = config_get_array_value(config, "POSICIONES_ENTRENADORES");
   char** array_objetivos = config_get_array_value(config, "OBJETIVOS_ENTRENADORES");
   char** array_pokemones = config_get_array_value(config, "POKEMON_ENTRENADORES");

   for (int i = 0; array_posiciones[i] != NULL; ++i) {
       t_entrenador* entrenador = malloc(sizeof(t_entrenador));
       t_coord* posicion = parse_posicion_entrenador(array_posiciones[i]);
       t_list* objetivos = parse_pokemones(array_objetivos[i]);
       t_list* pokemones = parse_pokemones(array_pokemones[i]);

       // TODO hacer un constructor en entrenador.c/h
       entrenador->thread_id = i;
       entrenador->posicion = posicion;
       entrenador->objetivos = objetivos;
       entrenador->pokemon_atrapados = pokemones;
       entrenador->estado = NEW;

       list_add(entrenadores, entrenador);

       free(array_posiciones[i]);
       free(array_objetivos[i]);
       free(array_pokemones[i]);
   }

   free(array_posiciones);
   free(array_objetivos);
   free(array_pokemones);

   return entrenadores;
}

static t_coord* parse_posicion_entrenador(char* posicion_entrenador){
    t_coord* coordenada = malloc(sizeof(t_coord));
    char** pos_splitted = string_split(posicion_entrenador, "|");
    coordenada->x = atoi(pos_splitted[0]);
    coordenada->y = atoi(pos_splitted[1]);

    free(pos_splitted[0]);
    free(pos_splitted[1]);
    free(pos_splitted);
    return coordenada;
}

static t_list* parse_pokemones(char* array_pokemones){
    t_list* pokemones = list_create();
    char** pok_splitted = string_split(array_pokemones, "|");
    int i = 0;
    while (pok_splitted[i] != NULL) {
        char* pokemon = pok_splitted[i];
        string_to_upper(pokemon);
        list_add(pokemones, pokemon);
        i++;
    }
    free(pok_splitted);
    return pokemones;
}

static void log_config_team(t_config_team* config_team){
    log_debug(default_logger, "Tiempo Reconexion: %i", config_team->tiempo_reconexion);
    log_debug(default_logger, "Retardo Ciclo CPU: %i", config_team->retardo_ciclo_cpu);
    log_debug(default_logger, "Algoritmo Planificacion: %s", config_team->algoritmo_planificacion);
    log_debug(default_logger, "Quantum: %i", config_team->quantum);
    log_debug(default_logger, "Estimacion Inicial: %i", config_team->estimacion_inicial);
    log_debug(default_logger, "IP Broker: %s", config_team->ip_broker);
    log_debug(default_logger, "IP Puerto: %s", config_team->puerto_broker);
    log_debug(default_logger, "Log File: %s", config_team->log_file);
}

void liberar_config_team(){
    config_destroy(config);
    free(config_team);
}

static t_log* iniciar_default_logger() {
    return log_create(default_logger_path, "DEFAULT", true, LOG_LEVEL_DEBUG);
}

static t_log* iniciar_team_logger(char* logger_path) {
    return log_create(logger_path, "TEAM", true, LOG_LEVEL_INFO);
}


