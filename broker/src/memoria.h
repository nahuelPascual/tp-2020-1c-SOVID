/*
 * memoria.h
 *
 *  Created on: 10 jun. 2020
 *      Author: utnso
 */

#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <math.h>

#include <delibird-commons/utils/paquete.h>
#include <delibird-commons/utils/list.h>

#include "utils.h"
#include "particion.h"
#include "logger.h"

typedef enum {
    PARTICIONES_DINAMICAS, BUDDY_SYSTEM
} t_algoritmo_memoria;

typedef enum {
    FIRST_FIT, BEST_FIT
} t_algoritmo_particion_libre;

typedef enum {
    FIFO, LRU
} t_algoritmo_reemplazo;

typedef struct {
    void* data;

    int tamanio_minimo_particion;
    t_algoritmo_memoria algoritmo_memoria;
    t_algoritmo_reemplazo algoritmo_reemplazo;
    t_algoritmo_particion_libre algoritmo_particion_libre;
    int frecuencia_compactacion;
    int contador_particiones_desocupadas;

    t_list* particiones;
} t_memoria;

t_memoria* memoria_crear(
    int tamanio_memoria,
    int tamanio_minimo_particion,
    t_algoritmo_memoria algoritmo_memoria,
    t_algoritmo_reemplazo algoritmo_reemplazo,
    t_algoritmo_particion_libre algoritmo_particion_libre,
    int frecuencia_compactacion
);

void* memoria_get_direccion_fisica(t_memoria* memoria, int direccion_logica);

bool memoria_existe_particion_libre_con(t_memoria* memoria, int tamanio_minimo);
t_particion* memoria_buscar_particion_libre_con(t_memoria* memoria, int tamanio_minimo);
void memoria_dividir_particion_si_es_mas_grande_que(t_memoria* memoria, t_particion* particion, int tamanio);
void memoria_asignar_paquete_a_la_particion(t_memoria* memoria, t_paquete* paquete, t_particion* particion);
t_particion* memoria_get_particion_a_desocupar(t_memoria* memoria);

void memoria_aumentar_contador_particiones_desocupadas(t_memoria* memoria);
void memoria_resetear_contador_particiones_desocupadas(t_memoria* memoria);

bool memoria_corresponde_compactar(t_memoria* memoria);
void memoria_compactar(t_memoria* memoria);
int memoria_consolidar(t_memoria* memoria);

#endif /* MEMORIA_H_ */
