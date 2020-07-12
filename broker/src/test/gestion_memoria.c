/*
 * gestion_memoria.c
 *
 *  Created on: 23 jun. 2020
 *      Author: utnso
 */

#ifndef TEST_GESTION_MEMORIA_C_
#define TEST_GESTION_MEMORIA_C_

#include "gestion_memoria.h"

void printear_particiones_actuales(t_memoria* memoria) {
    int i = 0;
    void _print(t_particion* particion) {
        printf("P%i:", i);
        printf("[%i,", particion->id_mensaje_asociado);
        printf("%i,", particion->base);
        printf("%i,", particion->tamanio);
        printf("%i,", particion->tiempo_carga);
        printf("%i,", particion->tiempo_ultima_referencia);
        printf("%s] ", particion->esta_libre ? "LI" : "OC");
        i++;
    }
    list_iterate(memoria->particiones, (void*) _print);
}

void probar_consolidacion(t_memoria* memoria) {

    printf("LISTA INICIAL\n");
    printear_particiones_actuales(memoria);
    printf("\n\n");

    memoria_consolidar(memoria);

    printf("LISTA DESPUES DE CONSOLIDAR\n");
    printear_particiones_actuales(memoria);
    printf("\n\n");

}

void probar_compactacion(t_memoria* memoria) {

    printf("LISTA INICIAL\n");
    printear_particiones_actuales(memoria);
    printf("\n\n");

    memoria_compactar(memoria);

    printf("LISTA DESPUES DE COMPACTAR\n");
    printear_particiones_actuales(memoria);
    printf("\n\n");
}

void test_memoria_sin_vaciado() {

    t_memoria* memoria = memoria_crear(0, 1024, PARTICIONES_DINAMICAS, FIRST_FIT, FIFO, 3);

    printf("1° TEST: Consolido lista previamente ordenada\n\n");

    list_clean_and_destroy_elements(memoria->particiones, (void*) memoria_liberar_particion);

    t_particion* particion0 = memoria_crear_particion(0, 10);
    t_particion* particion1 = memoria_crear_particion(10, 10);
    t_particion* particion2 = memoria_crear_particion(20, 10);
    t_particion* particion3 = memoria_crear_particion(30, 10);
    particion3->esta_libre = false;

    list_add(memoria->particiones, particion0);
    list_add(memoria->particiones, particion1);
    list_add(memoria->particiones, particion2);
    list_add(memoria->particiones, particion3);

    probar_consolidacion(memoria);

    list_clean_and_destroy_elements(memoria->particiones, (void*) memoria_liberar_particion);

    printf("2° TEST: No tengo que consolidar nada\n\n");

    t_particion* particion4 = memoria_crear_particion(0, 10);
    t_particion* particion5 = memoria_crear_particion(10, 10);
    particion5->esta_libre = false;
    t_particion* particion6 = memoria_crear_particion(20, 10);
    t_particion* particion7 = memoria_crear_particion(30, 10);
    particion7->esta_libre = false;

    list_add(memoria->particiones, particion4);
    list_add(memoria->particiones, particion5);
    list_add(memoria->particiones, particion6);
    list_add(memoria->particiones, particion7);

    probar_consolidacion(memoria);

    list_clean_and_destroy_elements(memoria->particiones, (void*) memoria_liberar_particion);

    printf("3° TEST: Consolido lista de una sola particion\n\n");

    t_particion* particion8 = memoria_crear_particion(0, 10);

    list_add(memoria->particiones, particion8);

    probar_consolidacion(memoria);

    list_clean_and_destroy_elements(memoria->particiones, (void*) memoria_liberar_particion);

    printf("4° TEST: Compacto una lista de particiones\n\n");

    t_particion* particion14 = memoria_crear_particion(0, 10);
    t_particion* particion15 = memoria_crear_particion(10, 10);
    particion15->esta_libre = false;
    t_particion* particion16 = memoria_crear_particion(20, 10);
    t_particion* particion17 = memoria_crear_particion(30, 10);
    t_particion* particion18 = memoria_crear_particion(40, 10);
    t_particion* particion19 = memoria_crear_particion(50, 10);

    list_add(memoria->particiones, particion14);
    list_add(memoria->particiones, particion15);
    list_add(memoria->particiones, particion16);
    list_add(memoria->particiones, particion17);
    list_add(memoria->particiones, particion18);
    list_add(memoria->particiones, particion19);

    probar_compactacion(memoria);

    list_clean_and_destroy_elements(memoria->particiones, (void*) memoria_liberar_particion);

    printf("5° TEST: Compacto lista que termina con una particion ocupada\n\n");

    t_particion* particion20 = memoria_crear_particion(0, 10);
    t_particion* particion21 = memoria_crear_particion(10, 10);
    particion21->esta_libre = false;
    t_particion* particion22 = memoria_crear_particion(20, 10);
    t_particion* particion23 = memoria_crear_particion(30, 10);
    particion23->esta_libre = false;
    t_particion* particion24 = memoria_crear_particion(40, 10);
    t_particion* particion25 = memoria_crear_particion(50, 10);
    particion25->esta_libre = false;

    list_add(memoria->particiones, particion20);
    list_add(memoria->particiones, particion21);
    list_add(memoria->particiones, particion22);
    list_add(memoria->particiones, particion23);
    list_add(memoria->particiones, particion24);
    list_add(memoria->particiones, particion25);

    probar_compactacion(memoria);

    list_clean_and_destroy_elements(memoria->particiones, (void*) memoria_liberar_particion);

    printf("6° TEST: Compacto lista que termina con particion libre\n\n");

    t_particion* particion26 = memoria_crear_particion(0, 10);
    particion26->esta_libre = false;
    t_particion* particion27 = memoria_crear_particion(10, 10);
    t_particion* particion28 = memoria_crear_particion(20, 10);
    particion28->esta_libre = false;
    t_particion* particion29 = memoria_crear_particion(30, 10);
    t_particion* particion30 = memoria_crear_particion(40, 10);
    particion30->esta_libre = false;
    t_particion* particion31 = memoria_crear_particion(50, 10);

    list_add(memoria->particiones, particion26);
    list_add(memoria->particiones, particion27);
    list_add(memoria->particiones, particion28);
    list_add(memoria->particiones, particion29);
    list_add(memoria->particiones, particion30);
    list_add(memoria->particiones, particion31);

    probar_compactacion(memoria);

    list_clean_and_destroy_elements(memoria->particiones, (void*) memoria_liberar_particion);

    printf("7° TEST: Recupero mensaje despues de compactar\n\n");

    t_particion* particion32 = memoria_crear_particion(0, 1);
    t_particion* particion33 = memoria_crear_particion(1, 30);
    t_particion* particion34 = memoria_crear_particion(31, 20);

    list_add(memoria->particiones, particion32);
    list_add(memoria->particiones, particion33);
    list_add(memoria->particiones, particion34);

    char* name = "Pikachu";
    t_new_pokemon* pokemon = mensaje_crear_new_pokemon(name, 15, 2, 1);
    t_paquete* paquete = paquete_from_new_pokemon(pokemon);

    memoria_asignar_paquete_a_la_particion(memoria, paquete, particion33);

    free(paquete->payload);

    probar_compactacion(memoria);

    paquete->payload = malloc(paquete->header->payload_size);

    memcpy(paquete->payload, memoria_get_direccion_fisica_de(memoria, particion33), paquete->header->payload_size);

    t_new_pokemon* pokemon_recuperado = paquete_to_new_pokemon(paquete);
    printf("Recupere un %s", pokemon_recuperado->nombre);

    list_clean_and_destroy_elements(memoria->particiones, (void*) memoria_liberar_particion);
}

void test_memoria_con_vaciado() {

    t_buzon* buzon = buzon_crear();

    char* name = "Pikachu";
    t_new_pokemon* pokemon = mensaje_crear_new_pokemon(name, 15, 2, 1);

    printf("1° TEST: En una memoria con espacio para 3 mensajes, cuando se llena tengo que\n"
           "         desocupar particiones por algoritmo FIFO\n\n");

    t_memoria* memoria1 = memoria_crear(0, 80, PARTICIONES_DINAMICAS, FIRST_FIT, FIFO, 3);
    buzon->memoria = memoria1;

    for(int i = 0; i < 5; i++) {
        t_paquete* paquete = paquete_from_new_pokemon(pokemon);
        buzon_almacenar_mensaje(buzon, paquete);
        printf("Lista al agregar mensaje n°%i: ", paquete->header->id_mensaje);
        printear_particiones_actuales(buzon->memoria);
        printf("\n");
        fflush(stdout);
    }
    printf("\n");

    printf("2° TEST: En una memoria con espacio para 3 mensajes, cuando se llena tengo que\n"
           "         desocupar particiones por algoritmo LRU\n\n");

    t_memoria* memoria2 = memoria_crear(0, 80, PARTICIONES_DINAMICAS, FIRST_FIT, LRU, 3);
    buzon->memoria = memoria2;

    for(int i = 0; i < 4; i++) {
        t_paquete* paquete = paquete_from_new_pokemon(pokemon);
        buzon_almacenar_mensaje(buzon, paquete);
        printf("Lista al agregar mensaje n°%i: ", paquete->header->id_mensaje);
        printear_particiones_actuales(buzon->memoria);
        printf("\n");
        fflush(stdout);

        if(i == 2){
            t_particion* primera_particion = list_get(buzon->memoria->particiones,0);
            memoria_get_direccion_fisica_de(buzon->memoria, primera_particion);
        }
    }
}

void test_buddy_system() {
    printf("TEST: la funcion recibe 2 particiones que son buddies\n\n");
    t_particion* particion1 = memoria_crear_particion(0, 32);
    t_particion* particion2 = memoria_crear_particion(32, 32);
    if(memoria_son_particiones_buddies(particion1, particion2))
        printf("Esta andando bien :)\n");
    else
        printf("La puta madre, no de nuevo :(\n");
    printf("\n");
    fflush(stdout);

    printf("TEST: la funcion recibe 2 particiones con distinto tamanio\n\n");
    t_particion* particion3 = memoria_crear_particion(0, 32);
    t_particion* particion4 = memoria_crear_particion(32, 64);
    if(!memoria_son_particiones_buddies(particion3, particion4))
        printf("Esta andando bien :)\n");
    else
        printf("La puta madre, no de nuevo :(\n");
    printf("\n");
    fflush(stdout);

    printf("TEST: la funcion recibe 2 particiones con el mismo tamanio pero no son buddies\n\n");
    t_particion* particion5 = memoria_crear_particion(0, 32);
    t_particion* particion6 = memoria_crear_particion(64, 32);
    if(!memoria_son_particiones_buddies(particion5, particion6))
        printf("Esta andando bien :)\n");
    else
        printf("La puta madre, no de nuevo :(\n");
    printf("\n");
    fflush(stdout);
}

#endif /* TEST_GESTION_MEMORIA_C_ */
