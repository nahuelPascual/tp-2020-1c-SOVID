/*
 * list.c
 *
 *  Created on: 1 jun. 2020
 *      Author: utnso
 */

#include "list.h"

bool list_equals(t_list* una_lista, t_list* otra_lista, bool (*comparador)(void*, void*)) {
    t_list* copia_otra_lista = list_duplicate(otra_lista);

    for(int i = 0; i < list_size(una_lista); i++) {

        void* un_elemento = list_get(una_lista, i);

        bool es_igual_a_un_elemento(void* otro_elemento) {
            return comparador(un_elemento, otro_elemento);
        }

        void* eliminado = list_remove_by_condition(copia_otra_lista, (void*) es_igual_a_un_elemento);

        if(!eliminado) {
            list_destroy(copia_otra_lista);
            return false;
        }
    }

    list_destroy(copia_otra_lista);
    return list_is_empty(copia_otra_lista);
}
