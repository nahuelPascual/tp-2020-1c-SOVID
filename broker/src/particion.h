/*
 * particion.h
 *
 *  Created on: 17 jul. 2020
 *      Author: utnso
 */

#ifndef PARTICION_H_
#define PARTICION_H_

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

typedef struct {
    bool esta_libre;

    int base;
    int tamanio;

    int tiempo_carga;
    int tiempo_ultima_referencia;

    uint32_t id_mensaje_asociado;
} t_particion;

t_particion* particion_crear(int base, int tamanio);
void particion_liberar(t_particion* particion);

void particion_ocupar(t_particion* particion);
void particion_desocupar(t_particion* particion);

bool particion_es_buddy_de(t_particion* particion, t_particion* otra_particion);

bool particion_esta_ocupada(t_particion* particion);
int particion_get_tiempo_carga(t_particion* particion);
int particion_get_tiempo_ultima_referencia(t_particion* particion);
int particion_get_tamanio(t_particion* particion);

#endif /* PARTICION_H_ */
