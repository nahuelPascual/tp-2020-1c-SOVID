/*
 * memoria.c
 *
 *  Created on: 9 jun. 2020
 *      Author: utnso
 */

#include "memoria.h"

t_particion* memoria_crear_particion(int base, int tamanio) {
    t_particion* particion = malloc(sizeof(t_particion));

    particion->esta_libre = true;
    particion->base = base;
    particion->tamanio = tamanio;

    return particion;
}

void memoria_liberar_particion(t_particion* particion) {
    free(particion);
}

t_memoria* memoria_crear(int tamanio_minimo_particion, int tamanio_maximo_memoria, t_tipo_algoritmo tipo_algoritmo) {
    t_memoria* memoria = malloc(sizeof(t_memoria));

    memoria->data = malloc(tamanio_maximo_memoria);
    memoria->tamanio_minimo_particion = tamanio_minimo_particion;
    memoria->tipo_algoritmo = tipo_algoritmo;
    memoria->particiones = list_create();
    list_add(memoria->particiones, memoria_crear_particion(0, tamanio_maximo_memoria));

    return memoria;
}

void* memoria_get_direccion_fisica(t_memoria* memoria, int direccion_logica) {
    return memoria->data + direccion_logica;
}

t_particion* memoria_buscar_particion_libre_con(t_memoria* memoria, int tamanio_minimo) {
    bool _es_particion_valida(t_particion* particion) {
      return particion->tamanio >= tamanio_minimo && particion->esta_libre;
    }

    switch(memoria->tipo_algoritmo) {
        case FIRST_FIT:
            return list_find(memoria->particiones, (void*) _es_particion_valida);
        default:
            return NULL;
    }
}

void memoria_dividir_particion_si_es_mas_grande_que(t_memoria* memoria, t_particion* particion, int tamanio) {
    if(particion->tamanio > tamanio) {
        int minimo_tamanio_posible = fmax(tamanio, memoria->tamanio_minimo_particion);

        t_particion* nueva_particion = memoria_crear_particion(
                particion->base + minimo_tamanio_posible,
                particion->tamanio - minimo_tamanio_posible);

        list_add(memoria->particiones, nueva_particion);

        particion->tamanio = minimo_tamanio_posible;
    }
}

void memoria_asignar_paquete_a_la_particion(t_memoria* memoria, t_paquete* paquete, t_particion* particion) {
    void* direccion_fisica = memoria_get_direccion_fisica(memoria, particion->base);

    memcpy(direccion_fisica, paquete->payload, paquete->header->payload_size);

    particion->esta_libre = false;
}
