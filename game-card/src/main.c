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
#include "filesystem.h"

t_listener_config* crear_gameboy_listener();
void liberar();
t_listener_config* crear_hilo_suscriptor(t_tipo_mensaje);
t_listener_config* get_pokemon_listener;
t_listener_config* new_pokemon_listener;
t_listener_config* catch_pokemon_listener;
t_listener_config* gameboy_config;

int main(int argc, char **argv) {
    config_game_card_init();

    get_pokemon_listener = crear_hilo_suscriptor(GET_POKEMON);
    new_pokemon_listener = crear_hilo_suscriptor(NEW_POKEMON);
    catch_pokemon_listener = crear_hilo_suscriptor(CATCH_POKEMON);
    gameboy_config = crear_gameboy_listener();

    filesystem_init();

    while(1){

    }
    liberar();
    return EXIT_SUCCESS;
}

t_listener_config* crear_hilo_suscriptor(t_tipo_mensaje mensaje) {
    t_listener_config* config = malloc(sizeof(t_listener_config));
    config->id = config_game_card->id;
    config->tipo_mensaje = mensaje;
    config->ip = config_game_card->ip_broker;
    config->puerto = config_game_card->puerto_broker;
    config->handler = escuchar_a;
    config->reintento_conexion = config_game_card->tiempo_reconexion;

    pthread_t thread;
    pthread_create(&thread, NULL, (void*)ipc_suscribirse_a, config);
    pthread_detach(thread);
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

void liberar(){
    liberar_config_game_card();
    log_destroy(logger);
    bitarray_destroy(bitarray);
    fclose(bitmap_file);
    liberar_t_path(metadata_dir_path);
    liberar_t_path(blocks_dir_path);
    liberar_t_path(files_dir_path);
    free(get_pokemon_listener);
    free(new_pokemon_listener);
    free(catch_pokemon_listener);
    free(gameboy_config);
}
