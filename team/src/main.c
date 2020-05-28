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
#include "subscripcion.h"
#include "sender.h"
#include "pokemon.h"
#include "configuration.h"
#include "objetivos.h"
#include "entrenador.h"
#include "planificador.h"

void crear_hilos_entrenador();
void crear_hilos_suscriptor();
void crear_gameboy_listener();

int main(int argc, char **argv) {
    config_team_init();
    t_dictionary* objetivos_globales = calcular_objetivos_globales(entrenador_get_all());

    crear_hilos_suscriptor();
    crear_gameboy_listener();

    init_pokemon_map();
    crear_hilos_entrenador();

    // se envia un get_pokemon por cada especie de pokemon requerida
    dictionary_iterator(objetivos_globales, (void*)enviar_get_pokemon);

    planificador_init();

    liberar_config_team();
    log_destroy(default_logger);
    log_destroy(logger);
  
    return EXIT_SUCCESS;
}

void crear_hilos_entrenador() {
    pthread_t hilos[entrenador_get_count()];
    void _crear_hilo(void* e) {
        t_entrenador* entrenador = (t_entrenador*) e;
        pthread_create(&hilos[entrenador->id], NULL, (void*)entrenador_execute, entrenador);
        pthread_detach(hilos[entrenador->id]);
    }
    list_iterate(entrenador_get_all(), (void*)_crear_hilo);
}

void crear_hilos_suscriptor() {
    suscribirse_a(LOCALIZED_POKEMON);
    suscribirse_a(APPEARED_POKEMON);
    suscribirse_a(CAUGHT_POKEMON);
}

void crear_gameboy_listener() {
    pthread_t gameboyListener;
    pthread_create(&gameboyListener, NULL, (void*)escuchar_gameboy, NULL);
    pthread_detach(gameboyListener);
}
