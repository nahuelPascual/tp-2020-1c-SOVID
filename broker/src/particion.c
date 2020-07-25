/*
 * particion.c
 *
 *  Created on: 17 jul. 2020
 *      Author: utnso
 */

#include "particion.h"

t_particion* particion_crear(int base, int tamanio) {
    t_particion* particion = malloc(sizeof(t_particion));

    particion->esta_libre = true;

    particion->base = base;
    particion->tamanio = tamanio;

    particion->tiempo_carga = 0;
    particion->tiempo_ultima_referencia = 0;

    particion->id_mensaje_asociado = 0;

    return particion;
}

void particion_liberar(t_particion* particion) {
    free(particion);
}

void particion_ocupar(t_particion* particion) {
    particion->esta_libre = false;
    particion->tiempo_carga = clock();
    particion->tiempo_ultima_referencia = clock();
}

void particion_desocupar(t_particion* particion) {
    particion->esta_libre = true;
    particion->tiempo_carga = 0;
    particion->tiempo_ultima_referencia = 0;
    particion->id_mensaje_asociado = 0;
}

bool particion_es_buddy_de(t_particion* particion, t_particion* otra_particion) {
    return particion->tamanio == otra_particion->tamanio && particion->base == (otra_particion->base ^ particion->tamanio)
            && otra_particion->base == (particion->base ^ otra_particion->tamanio);
}

bool particion_esta_ocupada(t_particion* particion) {
    return !particion->esta_libre;
}

int particion_get_tiempo_carga(t_particion* particion) {
    return particion->tiempo_carga;
}

int particion_get_tiempo_ultima_referencia(t_particion* particion) {
    return particion->tiempo_ultima_referencia;
}

int particion_get_tamanio(t_particion* particion) {
    return particion->tamanio;
}
