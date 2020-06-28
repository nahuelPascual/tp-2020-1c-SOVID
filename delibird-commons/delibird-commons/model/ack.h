/*
 * ack.h
 *
 *  Created on: 8 may. 2020
 *      Author: utnso
 */

#ifndef MODEL_ACK_H_
#define MODEL_ACK_H_

#include <stdlib.h>
#include <stdint.h>

typedef struct {
    uint32_t id_mensaje;
} t_ack;

t_ack* ack_crear(uint32_t id_mensaje);

void ack_liberar(t_ack* ack);

#endif /* MODEL_ACK_H_ */
