/*
 * main.c
 *
 *  Created on: Apr 8, 2020
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <delibird-commons/model/suscripcion.h>
#include "subscripcion.h"
#include "configuration.h"

t_listener_config* crear_suscripciones();
t_listener_config* crear_gameboy_listener();

int main(int argc, char **argv) {
    config_game_card_init();
    t_listener_config* suscripciones_config = crear_suscripciones();
    t_listener_config* gameboy_config = crear_gameboy_listener();
    while(1){

    }
    liberar_config_game_card();
    log_destroy(logger);
    free(suscripciones_config);
    free(gameboy_config);

    return EXIT_SUCCESS;
}

t_listener_config* crear_suscripciones() {
    t_listener_config* config = malloc(sizeof(t_listener_config));
    config->ip = config_game_card->ip_broker;
    config->puerto = config_game_card->puerto_broker;
    config->handler = escuchar_a;

    ipc_suscribirse_a(GET_POKEMON, config_game_card->id, tiempo_conexion_default, config);
    ipc_suscribirse_a(NEW_POKEMON, config_game_card->id, tiempo_conexion_default, config);
    ipc_suscribirse_a(CATCH_POKEMON, config_game_card->id, tiempo_conexion_default, config);

    return config;
}

t_listener_config* crear_gameboy_listener(){
    t_listener_config* config = malloc(sizeof(t_listener_config));
    config->ip = config_game_card->ip_game_card;
    config->puerto = config_game_card->puerto_game_card;
    config->handler = escuchar_a;

    ipc_crear_gameboy_listener(config);

    return config;
}
