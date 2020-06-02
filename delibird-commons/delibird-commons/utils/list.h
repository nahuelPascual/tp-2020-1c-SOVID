/*
 * list.h
 *
 *  Created on: 1 jun. 2020
 *      Author: utnso
 */

#ifndef UTILS_LIST_H_
#define UTILS_LIST_H_

#include <stdbool.h>

#include <commons/collections/list.h>

bool list_equals_by(t_list* una_lista, t_list* otra_lista, void* (*key_extractor)(void*));

#endif /* UTILS_LIST_H_ */
