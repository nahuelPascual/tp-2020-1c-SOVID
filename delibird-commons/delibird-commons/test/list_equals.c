/*
 * list_equals.c
 *
 *  Created on: 4 jun. 2020
 *      Author: utnso
 */

#include "list_equals.h"

bool igualigual(int un_elemento, int otro_elemento) {
    return un_elemento == otro_elemento;
}

void test_all() {
    t_list* una_lista = list_create();
    t_list* otra_lista = list_create();

    //DOS LISTAS IGUALES: [1,2,3] y [1,2,3]
    list_add(una_lista, (void*) 1);
    list_add(una_lista, (void*) 2);
    list_add(una_lista, (void*) 3);
    list_add(otra_lista, (void*) 1);
    list_add(otra_lista, (void*) 2);
    list_add(otra_lista, (void*) 3);
    printf("DOS LISTAS IGUALES: [1,2,3] y [1,2,3]: %s\n", list_equals(una_lista, otra_lista, (void*) igualigual) ? "SI" : "NO");
    list_clean(una_lista);
    list_clean(otra_lista);

    //DOS LISTAS IGUALES EN DISTINTO ORDEN: [1,2,3] y [2,3,1]
    list_add(una_lista, (void*) 1);
    list_add(una_lista, (void*) 2);
    list_add(una_lista, (void*) 3);
    list_add(otra_lista, (void*) 2);
    list_add(otra_lista, (void*) 3);
    list_add(otra_lista, (void*) 1);
    printf("DOS LISTAS IGUALES EN DISTINTO ORDEN: [1,2,3] y [2,3,1]: %s\n", list_equals(una_lista, otra_lista, (void*) igualigual) ? "SI" : "NO");
    list_clean(una_lista);
    list_clean(otra_lista);

    //DOS LISTAS COMPLETAMENTE DISTINTAS: [1,2,3] y [4,5,6]
    list_add(una_lista, (void*) 1);
    list_add(una_lista, (void*) 2);
    list_add(una_lista, (void*) 3);
    list_add(otra_lista, (void*) 4);
    list_add(otra_lista, (void*) 5);
    list_add(otra_lista, (void*) 6);
    printf("DOS LISTAS COMPLETAMENTE DISTINTAS: [1,2,3] y [4,5,6]: %s\n", list_equals(una_lista, otra_lista, (void*) igualigual) ? "SI" : "NO");
    list_clean(una_lista);
    list_clean(otra_lista);

    //DOS LISTAS IGUALES, CON REPETIDOS Y EN DISTINTO ORDEN: [1,1,2] y [2,1,1]
    list_add(una_lista, (void*) 1);
    list_add(una_lista, (void*) 1);
    list_add(una_lista, (void*) 2);
    list_add(otra_lista, (void*) 2);
    list_add(otra_lista, (void*) 1);
    list_add(otra_lista, (void*) 1);
    printf("DOS LISTAS IGUALES, CON REPETIDOS Y EN DISTINTO ORDEN: [1,1,2] y [2,1,1]: %s\n",
            list_equals(una_lista, otra_lista, (void*) igualigual) ? "SI" : "NO");
    list_clean(una_lista);
    list_clean(otra_lista);

    //OTROS CASOS: [1,2,3] y [1,1,1]
    list_add(una_lista, (void*) 1);
    list_add(una_lista, (void*) 2);
    list_add(una_lista, (void*) 3);
    list_add(otra_lista, (void*) 1);
    list_add(otra_lista, (void*) 1);
    list_add(otra_lista, (void*) 1);
    printf("OTROS CASOS: [1,2,3] y [1,1,1]: %s\n", list_equals(una_lista, otra_lista, (void*) igualigual) ? "SI" : "NO");
    list_clean(una_lista);
    list_clean(otra_lista);

    //OTROS CASOS: [1,2,3] y [1]
    list_add(una_lista, (void*) 1);
    list_add(una_lista, (void*) 2);
    list_add(una_lista, (void*) 3);
    list_add(otra_lista, (void*) 1);
    printf("OTROS CASOS: [1,2,3] y [1]: %s\n", list_equals(una_lista, otra_lista, (void*) igualigual) ? "SI" : "NO");
    list_clean(una_lista);
    list_clean(otra_lista);

    //OTROS CASOS: [1,1,1] y [1,2,3]
    list_add(una_lista, (void*) 1);
    list_add(una_lista, (void*) 1);
    list_add(una_lista, (void*) 1);
    list_add(otra_lista, (void*) 1);
    list_add(otra_lista, (void*) 2);
    list_add(otra_lista, (void*) 3);
    printf("OTROS CASOS: [1,1,1] y [1,2,3]: %s\n", list_equals(una_lista, otra_lista, (void*) igualigual) ? "SI" : "NO");
    list_clean(una_lista);
    list_clean(otra_lista);

    //OTROS CASOS: [1] y [1,2,3]
    list_add(una_lista, (void*) 1);
    list_add(otra_lista, (void*) 1);
    list_add(otra_lista, (void*) 2);
    list_add(otra_lista, (void*) 3);
    printf("OTROS CASOS: [1] y [1,2,3]: %s\n", list_equals(una_lista, otra_lista, (void*) igualigual) ? "SI" : "NO");
    list_destroy(una_lista);
    list_destroy(otra_lista);
}
