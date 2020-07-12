/*
 * main.c
 *
 *  Created on: Apr 8, 2020
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <commons/collections/dictionary.h>
#include <commons/log.h>
#include "subscripcion.h"
#include "sender.h"
#include "pokemon.h"
#include "configuration.h"
#include "objetivos.h"
#include "entrenador.h"
#include "planificador.h"
#include "delibird-commons/model/suscripcion.h"

t_log* logger;
t_log* default_logger;

void crear_hilos_entrenador();
t_listener_config* crear_hilos_suscriptor();
t_listener_config* crear_gameboy_listener();
void verificar_estado_inicial_entrenadores();
void enviar_gets_al_broker();

int main(int argc, char **argv) {
    config_team_init();
    objetivos_globales_init(entrenador_get_all());
    sender_init_mensajes_esperando_respuesta();
    init_pokemon_map();
    crear_hilos_entrenador();
    pthread_t planificador = planificador_init();

    verificar_estado_inicial_entrenadores();

    t_listener_config* suscriptor_config = crear_hilos_suscriptor();
    t_listener_config* gameboy_config = crear_gameboy_listener();

    enviar_gets_al_broker();

    pthread_join(planificador, NULL);

    liberar_config_team();
    log_destroy(default_logger);
    log_destroy(logger);
    free(suscriptor_config);
    free(gameboy_config);
  
    return EXIT_SUCCESS;
}

void crear_hilos_entrenador() {
    pthread_t hilos[entrenador_get_count()];
    log_debug(default_logger, "Creando %d hilos entrenador", entrenador_get_count());
    void _crear_hilo(void* e) {
        t_entrenador* entrenador = (t_entrenador*) e;
        pthread_create(&hilos[entrenador->id], NULL, (void*)entrenador_execute, entrenador);
        pthread_detach(hilos[entrenador->id]);
    }
    list_iterate(entrenador_get_all(), (void*)_crear_hilo);
}

t_listener_config* crear_hilos_suscriptor() {
    t_listener_config* config = malloc(sizeof(t_listener_config));
    config->ip = config_team->ip_broker;
    config->puerto = config_team->puerto_broker;
    config->handler = escuchar_a;

    ipc_suscribirse_a(LOCALIZED_POKEMON, config_team->id, tiempo_conexion_default, config);
    ipc_suscribirse_a(APPEARED_POKEMON, config_team->id, tiempo_conexion_default, config);
    ipc_suscribirse_a(CAUGHT_POKEMON, config_team->id, tiempo_conexion_default, config);
    return config;
}

t_listener_config* crear_gameboy_listener(){
    t_listener_config* config = malloc(sizeof(t_listener_config));
    config->ip = config_team->ip_team;
    config->puerto = config_team->puerto_team;
    config->handler = escuchar_a;

    ipc_crear_gameboy_listener(config);
    return config;
}

void verificar_estado_inicial_entrenadores() {
    list_iterate(entrenador_get_all(), (void*)entrenador_verificar_objetivos);
    list_iterate(entrenador_get_all(), (void*)planificador_verificar_deadlock_exit);
}

void enviar_gets_al_broker() {
    t_list* objetivos = objetivos_get_especies_pendientes();
    list_iterate(objetivos, (void*)enviar_get_pokemon);
    list_destroy(objetivos);
}
