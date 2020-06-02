/*
 * list.c
 *
 *  Created on: 1 jun. 2020
 *      Author: utnso
 */

#include "list.h"

bool list_equals_by(t_list* una_lista, t_list* otra_lista, void* (*key_extractor)(void*)) {
    if(list_size(una_lista) != list_size(otra_lista))
        return false;

    bool _order_comparator(void* un_elemento, void* otro_elemento) {
        return key_extractor(un_elemento) < key_extractor(otro_elemento);
    }
    bool _equality_comparator(void* un_elemento, void* otro_elemento) {
        return key_extractor(un_elemento) == key_extractor(otro_elemento);
    }

    t_list* una_lista_ordenada = list_sorted(una_lista, (void*) _order_comparator);
    t_list* otra_lista_ordenada = list_sorted(otra_lista, (void*) _order_comparator);

    int i = 0;
    bool listas_iguales = true;
    while(listas_iguales && i < list_size(una_lista)) {
        void* un_elemento = list_get(una_lista_ordenada, i);
        void* otro_elemento = list_get(otra_lista_ordenada, i);

        listas_iguales = listas_iguales && _equality_comparator(un_elemento, otro_elemento);
        i++;
    }

    return listas_iguales;
}
