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

typedef enum {
    FIRST_FIT, BEST_FIT
} t_tipo_algoritmo;

typedef struct {
  bool esta_libre;
  int base;
  int tamanio;
} t_particion;

typedef struct {
    void* data;
    int tamanio_minimo_particion;
    t_tipo_algoritmo tipo_algoritmo;
    t_list* particiones;
} t_memoria;

t_particion* memoria_crear_particion(int base, int tamanio);
void memoria_liberar_particion(t_particion* particion);

t_memoria* memoria_crear(int tamanio_minimo_particion, int tamanio_maximo_memoria, t_tipo_algoritmo tipo_algoritmo);

void* memoria_get_direccion_fisica(t_memoria* memoria, int direccion_logica);

t_particion* memoria_buscar_particion_libre_con(t_memoria* memoria, int tamanio_minimo);
void memoria_dividir_particion_si_es_mas_grande_que(t_memoria* memoria, t_particion* particion, int tamanio);
void memoria_asignar_paquete_a_la_particion(t_memoria* memoria, t_paquete* paquete, t_particion* particion);

#endif /* MEMORIA_H_ */
