/*
 * list_min_by.c
 *
 *  Created on: 6 jul. 2020
 *      Author: utnso
 */

#include "list_min_by.h"

t_test* crear_t_test(int num1, int num2) {
    t_test* test = malloc(sizeof(t_test));
    test->atributo1 = num1;
    test->atributo2 = num2;
    return test;
}

void test_list_min_by(){

    t_list* lista = list_create();
    list_add(lista, crear_t_test(3,3));
    list_add(lista, crear_t_test(2,2));
    list_add(lista, crear_t_test(4,4));
    list_add(lista, crear_t_test(2,2));
    list_add(lista, crear_t_test(1,1));

    int _get_atributo1(t_test* test) {
        return test->atributo1;
    }

    t_test* minimo = list_min_by(lista, (void*)_get_atributo1);
    printf("El minimo de la lista es: %i\n", minimo->atributo1);

}
