/*
 * objetivos.h
 *
 *  Created on: 10 may. 2020
 *      Author: utnso
 */

#ifndef UTILS_OBJETIVOS_H_
#define UTILS_OBJETIVOS_H_

#include <commons/collections/list.h>
#include "sender.h"

pthread_mutex_t mx_atrapados;

t_dictionary* objetivos_globales_init(t_list* entrenadores);
int objetivos_cantidad_pendientes(char* nombre);
t_list* objetivos_get_especies_pendientes();
void objetivos_capturado(char* nombre);

#endif /* UTILS_OBJETIVOS_H_ */
