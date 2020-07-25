/*
 * suscriptor.c
 *
 *  Created on: 24 jun. 2020
 *      Author: utnso
 */

#include "suscriptor.h"

t_suscriptor* suscriptor_crear(uint32_t id, t_tipo_mensaje tipo_mensaje, int socket_asociado) {
    t_suscriptor* suscriptor = malloc(sizeof(t_suscriptor));
    suscriptor->id = id;
    suscriptor->tipo_mensaje = tipo_mensaje;
    suscriptor->socket_asociado = socket_asociado;

    return suscriptor;
}

void suscriptor_liberar(t_suscriptor* suscriptor) {
    free(suscriptor);
}
