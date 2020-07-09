//
// Created by utnso on 9/05/20.
//

#ifndef TEAM_SENDER_H
#define TEAM_SENDER_H

#include <stdbool.h>
#include <delibird-commons/utils/ipc.h>
#include <delibird-commons/utils/logger.h>
#include "pokemon.h"
#include "configuration.h"

typedef struct {
    int id_entrenador;
    int id_mensaje;
    void* mensaje_enviado;
} t_mensaje_enviado;

void sender_init_mensajes_esperando_respuesta();
void enviar_get_pokemon(char* pokemon, void* cantidad);
bool enviar_catch_pokemon(int id_entrenador, t_pokemon_mapeado* pokemon);
bool is_respuesta_esperada(int id_mensaje_correlativo);
t_mensaje_enviado* get_mensaje_enviado(int id_mensaje);
void liberar_get_pokemon_enviado(t_mensaje_enviado*);
void liberar_catch_pokemon_enviado(t_mensaje_enviado*);

#endif //TEAM_SENDER_H
