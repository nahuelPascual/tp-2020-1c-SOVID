/*
 * list_index_of.c
 *
 *  Created on: 8 jul. 2020
 *      Author: utnso
 */

#include "list_index_of.h"

t_persona* _crear_persona(char* nombre, int edad) {
    t_persona* persona = malloc(sizeof(t_persona));
    persona->nombre = nombre;
    persona->edad = edad;
    return persona;
}

void test_list_index_of() {
    t_list* lista = list_create();

    //UNA LISTA VACIA
    printf("UNA LISTA VACIA: %i\n", list_index_of(lista, (void*) 1));
    list_clean(lista);

    //UNA LISTA SIN EL ELEMENTO
    list_add(lista, (void*) 1);
    list_add(lista, (void*) 2);
    list_add(lista, (void*) 3);
    printf("UNA LISTA SIN EL ELEMENTO: %i\n", list_index_of(lista, (void*) 4));
    list_clean(lista);

    //UNA LISTA CON EL ELEMENTO
    list_add(lista, (void*) 1);
    list_add(lista, (void*) 2);
    list_add(lista, (void*) 3);
    printf("UNA LISTA CON EL ELEMENTO: %i\n", list_index_of(lista, (void*) 3));
    list_clean(lista);

    //UNA LISTA CON EL ELEMENTO (TIPO COMPLEJO)
    t_persona* persona1 = _crear_persona("Micho", 10);
    t_persona* persona2 = _crear_persona("Tito", 20);
    t_persona* persona3 = _crear_persona("Negro", 30);

    list_add(lista, persona1);
    list_add(lista, persona2);
    printf("UNA LISTA CON EL ELEMENTO (TIPO COMPLEJO) - EL QUE ESTA: %i\n", list_index_of(lista, persona1));
    printf("UNA LISTA CON EL ELEMENTO (TIPO COMPLEJO) - EL QUE NO ESTA: %i\n", list_index_of(lista, persona3));
    list_destroy_and_destroy_elements(lista, free);
}
