/*
 * configuracion.h
 *
 *  Created on: 14 jul. 2020
 *      Author: utnso
 */

#ifndef CONFIGURACION_H_
#define CONFIGURACION_H_

#include <commons/config.h>
#include <commons/string.h>

#include "memoria.h"

typedef struct {
    int tamanio_memoria;
    int tamanio_minimo_particion;
    t_algoritmo_memoria algoritmo_memoria;
    t_algoritmo_reemplazo algoritmo_reemplazo;
    t_algoritmo_particion_libre algoritmo_particion_libre;
    char* ip_broker;
    char* puerto_broker;
    int frecuencia_compactacion;
    char* log_file;
    char* dump_file;
} t_configuracion;

t_configuracion* configuracion_crear();

#endif /* CONFIGURACION_H_ */
