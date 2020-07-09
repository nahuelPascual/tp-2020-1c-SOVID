/*
 * configuration.c
 *
 *  Created on: 31 may. 2020
 *      Author: utnso
 */

#include "configuration.h"

static char* const default_logger_path = "default.log";
static char* const config_path = "game_card.config";
static t_config* config;

static void log_config_game_card(t_config_game_card*);

void config_game_card_init() {
    logger = log_create(default_logger_path, "DEFAULT", true, LOG_LEVEL_DEBUG);

    config_game_card = malloc(sizeof(t_config_game_card));
    config = config_create(config_path);

    config_game_card->id = (uint32_t) config_get_int_value(config, "ID");
    config_game_card->tiempo_reconexion = config_get_int_value(config, "TIEMPO_DE_REINTENTO_CONEXION");
    config_game_card->tiempo_reintento_operacion = config_get_int_value(config, "TIEMPO_DE_REINTENTO_OPERACION");
    config_game_card->tiempo_retardo_operacion = config_get_int_value(config, "TIEMPO_RETARDO_OPERACION");
    config_game_card->punto_montaje_tall_grass = config_get_string_value(config, "PUNTO_MONTAJE_TALLGRASS");
    config_game_card->ip_broker = config_get_string_value(config, "IP_BROKER");
    config_game_card->puerto_broker = config_get_string_value(config, "PUERTO_BROKER");
    config_game_card->ip_game_card = config_get_string_value(config, "IP_GAME_CARD");
    config_game_card->puerto_game_card = config_get_string_value(config, "PUERTO_GAME_CARD");

    log_config_game_card(config_game_card); // logeo la config
}


static void log_config_game_card(t_config_game_card* config_game_card){
    log_debug(logger, "Game Card ID: %d", config_game_card->id);
    log_debug(logger, "Tiempo Reconexion: %i", config_game_card->tiempo_reconexion);
    log_debug(logger, "Tiempo Reintento Operacion: %i", config_game_card->tiempo_reintento_operacion);
    log_debug(logger, "Tiempo Retardo Operacion: %i", config_game_card->tiempo_retardo_operacion);
    log_debug(logger, "Punto Montaje Tall Grass: %s", config_game_card->punto_montaje_tall_grass);
    log_debug(logger, "IP Broker: %s", config_game_card->ip_broker);
    log_debug(logger, "Puerto Broker: %s", config_game_card->puerto_broker);
    log_debug(logger, "IP Game Card: %s", config_game_card->ip_game_card);
    log_debug(logger, "Puerto Game Card: %s", config_game_card->puerto_game_card);
}

void liberar_config_game_card(){
    config_destroy(config);
    free(config_game_card);
}
