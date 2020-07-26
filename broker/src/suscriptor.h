/*
 * suscriptor.h
 *
 *  Created on: 24 jun. 2020
 *      Author: utnso
 */

#ifndef SUSCRIPTOR_H_
#define SUSCRIPTOR_H_

#include <delibird-commons/model/mensaje.h>

typedef struct {
    uint32_t id;
    t_tipo_mensaje tipo_mensaje;
    int socket_asociado;
} t_suscriptor;

t_suscriptor* suscriptor_crear(uint32_t id, t_tipo_mensaje tipo_mensaje, int socket_asociado);
void suscriptor_liberar(t_suscriptor* suscriptor);

#endif /* SUSCRIPTOR_H_ */
