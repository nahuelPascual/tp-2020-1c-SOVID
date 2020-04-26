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

typedef struct {
    uint32_t tipo_mensaje;
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

t_paquete* paquete_from_new_pokemon(t_new_pokemon* pokemon);
t_paquete* paquete_from_localized_pokemon(t_localized_pokemon* pokemon);
t_paquete* paquete_from_get_pokemon(t_get_pokemon* pokemon);
t_paquete* paquete_from_appeared_pokemon(t_appeared_pokemon* pokemon);
t_paquete* paquete_from_catch_pokemon(t_catch_pokemon* pokemon);
t_paquete* paquete_from_caught_pokemon(t_caught_pokemon* pokemon);

#endif //PAQUETE_H
