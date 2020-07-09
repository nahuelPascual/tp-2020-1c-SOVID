/*
 * suscripcion.c
 *
 *  Created on: 8 may. 2020
 *      Author: utnso
 */

#include "suscripcion.h"

const uint32_t tiempo_conexion_default = 0;

t_suscripcion* suscripcion_crear(t_tipo_mensaje tipo_mensaje, uint32_t id_suscriptor, uint32_t tiempo) {
    t_suscripcion* suscripcion = malloc(sizeof(t_suscripcion));

    suscripcion->tipo_mensaje = tipo_mensaje;
    suscripcion->id_suscriptor = id_suscriptor;
    suscripcion->tiempo = tiempo;

    return suscripcion;
}

void suscripcion_liberar(t_suscripcion* suscripcion) {
    free(suscripcion);
}
