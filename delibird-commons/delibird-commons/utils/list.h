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

bool list_equals(t_list* una_lista, t_list* otra_lista, bool (*comparador)(void*, void*));

void* list_min_by(t_list* lista, void* key_extractor(void*));

int list_index_of(t_list* lista, void* elemento_a_buscar);

int list_sum_by(t_list* lista, int (*key_extractor)(void*));

#endif /* UTILS_LIST_H_ */
