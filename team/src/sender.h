//
// Created by utnso on 9/05/20.
//

#ifndef TEAM_SENDER_H
#define TEAM_SENDER_H

#include <stdbool.h>
#include <commons/collections/list.h>
#include <delibird-commons/model/mensaje.h>
#include <delibird-commons/utils/ipc.h>
#include <delibird-commons/utils/paquete.h>
#include "entrenador.h"

typedef struct {
    t_entrenador* entrenador;
    int id_mensaje;
    t_catch_pokemon* mensaje_enviado;
} t_captura;

void enviar_get_pokemon(char* pokemon, void* cantidad);
bool is_respuesta_esperada(int id_mensaje_correlativo);
t_captura* get_mensaje_enviado(int id_mensaje);

#endif //TEAM_SENDER_H
