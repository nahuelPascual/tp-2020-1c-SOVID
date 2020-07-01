/*
 * administrador_colas.h
 *
 *  Created on: 17 jun. 2020
 *      Author: utnso
 */

#ifndef ADMINISTRADOR_COLAS_H_
#define ADMINISTRADOR_COLAS_H_

#include <pthread.h>

#include <commons/collections/dictionary.h>

#include "cola.h"

typedef struct {
    t_dictionary* colas;
    int id_mensaje;

    pthread_mutex_t mutex_id_mensaje;
} t_administrador_colas;

t_administrador_colas* administrador_colas_crear();

t_cola* administrador_colas_get_cola_from(t_administrador_colas* administrador_colas, t_tipo_mensaje tipo_mensaje);

void administrador_colas_asignar_id_mensaje_a(t_administrador_colas* administrador_colas, t_paquete* paquete);

t_mensaje_despachable* administrador_colas_find_mensaje_despachable_by_id(t_administrador_colas* administrador_colas, uint32_t id_mensaje);

#endif /* ADMINISTRADOR_COLAS_H_ */
