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
#include "utils/parser.h"
#include "utils/objetivos.h"
#include "entrenador.h"
#include "planificador.h"

int main(int argc, char **argv) {
    t_config_team* config_team = parser_get_config_team();
    t_dictionary* objetivos_globales = calcular_objetivos_globales(entrenador_all());
	pthread_t suscriptorThread, gameboyListener, planificador;

	// TODO no usar este hilo extra; crear las tres suscripciones en el main
    pthread_create(&suscriptorThread, NULL, (void*)suscribirseAlBroker, NULL);

    int server = ipc_escuchar_en(config_team->ip_broker, config_team->puerto_broker);
    int gameboy = ipc_esperar_cliente(server);
    pthread_create(&gameboyListener, NULL, (void*)escuchar, gameboy);

    // se envia un get_pokemon por cada especie de pokemon
    dictionary_iterator(objetivos_globales, (void*)enviar_get_pokemon);

    init_pokemon_map();
    planificador_init();
    pthread_create(&planificador, NULL, (void*)planificar, NULL);

    pthread_join(gameboyListener, NULL);
    pthread_join(suscriptorThread, NULL);

    liberar_config_team(config_team);
    log_destroy(default_logger);
    log_destroy(logger);

    return EXIT_SUCCESS;
}
