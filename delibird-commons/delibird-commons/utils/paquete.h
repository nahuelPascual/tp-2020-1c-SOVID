//
// Created by utnso on 21/04/20.
//

#ifndef PAQUETE_H
#define PAQUETE_H

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <commons/collections/list.h>
#include "../model/mensaje.h"
#include "../model/ack.h"
#include "../model/suscripcion.h"
#include "../model/informe_id.h"

typedef enum {
    MENSAJE, ACK, SUSCRIPCION, INFORME_ID
} t_tipo_paquete;

typedef struct {
    t_tipo_paquete tipo_paquete;
    t_tipo_mensaje tipo_mensaje;
    uint32_t id_mensaje;
    uint32_t correlation_id_mensaje;
    uint32_t payload_size;
} t_header;

typedef struct {
    t_header* header;
    void* payload;
} t_paquete;

void paquete_liberar(t_paquete* paquete);

t_new_pokemon* paquete_to_new_pokemon(t_paquete* paquete);
t_localized_pokemon* paquete_to_localized_pokemon(t_paquete* paquete);
t_get_pokemon* paquete_to_get_pokemon(t_paquete* paquete);
t_appeared_pokemon* paquete_to_appeared_pokemon(t_paquete* paquete);
t_catch_pokemon* paquete_to_catch_pokemon(t_paquete* paquete);
t_caught_pokemon* paquete_to_caught_pokemon(t_paquete* paquete);
t_ack* paquete_to_ack(t_paquete* paquete);
t_suscripcion* paquete_to_suscripcion(t_paquete* paquete);
t_informe_id* paquete_to_informe_id(t_paquete* paquete);

t_paquete* paquete_from_new_pokemon(t_new_pokemon* pokemon);
t_paquete* paquete_from_localized_pokemon(t_localized_pokemon* pokemon);
t_paquete* paquete_from_get_pokemon(t_get_pokemon* pokemon);
t_paquete* paquete_from_appeared_pokemon(t_appeared_pokemon* pokemon);
t_paquete* paquete_from_catch_pokemon(t_catch_pokemon* pokemon);
t_paquete* paquete_from_caught_pokemon(t_caught_pokemon* pokemon);
t_paquete* paquete_from_ack(t_ack* ack);
t_paquete* paquete_from_suscripcion(t_suscripcion* suscripcion);
t_paquete* paquete_from_informe_id(t_informe_id* informe_id);

bool paquete_es_mensaje_de_respuesta(t_paquete* paquete);

#endif //PAQUETE_H
