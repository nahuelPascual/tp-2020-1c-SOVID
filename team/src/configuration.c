/*
 * configuration.c
 *
 *  Created on: 10 may. 2020
 *      Author: utnso
 */

#include "configuration.h"

extern t_log* default_logger;
extern t_log* logger;

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
static void verficar_objetivos(t_entrenador* e);

void config_team_init() {
    default_logger = iniciar_default_logger();

    config_team = malloc(sizeof(t_config_team));
    leer_config();

    config_team->id = (uint32_t) config_get_int_value(config, "ID");
    config_team->tiempo_reconexion = config_get_int_value(config, "TIEMPO_RECONEXION");
    config_team->retardo_ciclo_cpu = config_get_int_value(config, "RETARDO_CICLO_CPU");
    config_team->algoritmo_planificacion = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
    config_team->quantum = config_get_int_value(config, "QUANTUM");
    config_team->estimacion_inicial = config_get_int_value(config, "ESTIMACION_INICIAL");
    config_team->alpha = config_get_double_value(config, "ALPHA");
    config_team->ip_broker = config_get_string_value(config, "IP_BROKER");
    config_team->puerto_broker = config_get_string_value(config, "PUERTO_BROKER");
    config_team->ip_team = config_get_string_value(config, "IP_TEAM");
    config_team->puerto_team = config_get_string_value(config, "PUERTO_TEAM");
    config_team->log_file = config_get_string_value(config, "LOG_FILE");

    logger = iniciar_team_logger(config_team->log_file);

    log_config_team(config_team);

    t_list* entrenadores = parse_entrenadores();
    log_entrenadores(entrenadores);
    entrenador_init_list(entrenadores);
    list_destroy(entrenadores);
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
       t_coord* posicion = parse_posicion_entrenador(array_posiciones[i]);
       t_list* objetivos = parse_pokemones(array_objetivos[i]);
       t_list* pokemones = parse_pokemones(array_pokemones[i]);

       t_entrenador* entrenador = entrenador_new(i, objetivos, pokemones, posicion);

       list_add(entrenadores, entrenador);

       free(array_posiciones[i]);
       free(array_objetivos[i]);
       free(array_pokemones[i]);

       verficar_objetivos(entrenador);
   }

   free(array_posiciones);
   free(array_objetivos);
   free(array_pokemones);

   return entrenadores;
}

static void verficar_objetivos(t_entrenador* e) {
    void _marcar_capturado(void* elemento) {
        t_pokemon_capturado* capturado = (t_pokemon_capturado*) elemento;

        bool _condition(void* _elemento) {
            t_pokemon_objetivo* objetivo = (t_pokemon_objetivo*) _elemento;
            return !objetivo->fue_capturado && string_equals_ignore_case(objetivo->nombre, capturado->nombre);
        }
        t_pokemon_objetivo* objetivo = list_find(e->objetivos, (void*)_condition);

        if (objetivo != NULL) {
            objetivo->fue_capturado = true;
            capturado->es_objetivo = true;
        }
    }
    list_iterate(e->capturados, (void*)_marcar_capturado);
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

    if (array_pokemones == NULL) return pokemones;

    char** pok_splitted = string_split(array_pokemones, "|");
    int i = 0;
    while (pok_splitted[i] != NULL) {
        char* nombre = pok_splitted[i];
        string_to_upper(nombre);
        t_pokemon_objetivo* pokemon = malloc(sizeof(t_pokemon_capturado)); // capturado y objetivo son estructuralmente identicos pero tienen una semantica diferente
        pokemon->nombre = nombre;
        pokemon->fue_capturado = false;
        list_add(pokemones, pokemon);
        i++;
    }
    free(pok_splitted);

    return pokemones;
}

static void log_config_team(t_config_team* config_team){
    log_debug(default_logger, "Team ID: %d", config_team->id);
    log_debug(default_logger, "Tiempo Reconexion: %i", config_team->tiempo_reconexion);
    log_debug(default_logger, "Retardo Ciclo CPU: %i", config_team->retardo_ciclo_cpu);
    log_debug(default_logger, "Algoritmo Planificacion: %s", config_team->algoritmo_planificacion);
    log_debug(default_logger, "Quantum: %i", config_team->quantum);
    log_debug(default_logger, "Estimacion Inicial: %i", config_team->estimacion_inicial);
    log_debug(default_logger, "IP Broker: %s", config_team->ip_broker);
    log_debug(default_logger, "IP Puerto: %s", config_team->puerto_broker);
    log_debug(default_logger, "IP Team: %s", config_team->ip_team);
    log_debug(default_logger, "IP Team: %s", config_team->puerto_team);
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
