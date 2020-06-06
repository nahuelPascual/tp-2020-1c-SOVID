/*
 * objetivos.h
 *
 *  Created on: 10 may. 2020
 *      Author: utnso
 */

#ifndef UTILS_OBJETIVOS_H_
#define UTILS_OBJETIVOS_H_

#include <commons/collections/list.h>
#include "configuration.h"

t_dictionary* calcular_objetivos_globales(t_list* entrenadores);
bool is_pokemon_requerido(char* nombre);
t_list* objetivos_get_especies();
void objetivos_descontar_requeridos(char* nombre);

#endif /* UTILS_OBJETIVOS_H_ */
