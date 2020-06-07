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
#include <delibird-commons/model/ack.h>
#include <delibird-commons/model/suscripcion.h>
#include "pokemon.h"
#include "configuration.h"

typedef struct {
    int id_entrenador;
    int id_mensaje;
    t_catch_pokemon* mensaje_enviado;
} t_captura;

void sender_init_capturas_pendientes();
int enviar_suscripcion(t_tipo_mensaje tipo_mensaje);
void enviar_get_pokemon(char* pokemon, void* cantidad);
void enviar_catch_pokemon(int id_entrenador, t_pokemon_mapeado* pokemon);
void enviar_ack(uint32_t id_mensaje, int socket);
bool is_respuesta_esperada(int id_mensaje_correlativo);
t_captura* get_mensaje_enviado(int id_mensaje);
void liberar_captura(t_captura*);

#endif //TEAM_SENDER_H
