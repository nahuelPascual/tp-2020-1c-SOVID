/*
 * configuration.h
 *
 *  Created on: 31 may. 2020
 *      Author: utnso
 */

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <stdlib.h>
#include <stdint.h>
#include <commons/log.h>
#include <commons/config.h>

typedef struct {
    uint32_t id;
    int tiempo_reconexion;
    int tiempo_reintento_operacion;
    int tiempo_retardo_operacion;
    char* punto_montaje_tall_grass;
    char* ip_broker;
    char* puerto_broker;
    char* ip_game_card;
    char* puerto_game_card;
    int block_size;
    int blocks;
    char* magic_number;
} t_config_game_card;

t_log* logger;
t_config_game_card* config_game_card;

void config_game_card_init();
void liberar_config_game_card();

#endif /* CONFIGURATION_H_ */
