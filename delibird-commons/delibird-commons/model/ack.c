/*
 * ack.c
 *
 *  Created on: 8 may. 2020
 *      Author: utnso
 */

#include "ack.h"

t_ack* ack_crear(uint32_t id_suscriptor, uint32_t id_mensaje) {
    t_ack* ack = malloc(sizeof(t_ack));

    ack->id_suscriptor = id_suscriptor;
    ack->id_mensaje = id_mensaje;

    return ack;
}

void ack_liberar(t_ack* ack) {
    free(ack);
}
