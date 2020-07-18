/*
 * buzon.h
 *
 *  Created on: 18 jun. 2020
 *      Author: utnso
 */

#ifndef BUZON_H_
#define BUZON_H_

#include "memoria.h"
#include "administrador_colas.h"

typedef struct {
    t_administrador_colas* administrador_colas;
    t_memoria* memoria;

    pthread_mutex_t mutex_memoria;
} t_buzon;

t_buzon* buzon_crear(
    int tamanio_memoria,
    int tamanio_minimo_particion,
    t_algoritmo_memoria algoritmo_memoria,
    t_algoritmo_reemplazo algoritmo_reemplazo,
    t_algoritmo_particion_libre algoritmo_particion_libre,
    int frecuencia_compactacion
);

t_mensaje_despachable* buzon_almacenar_mensaje(t_buzon* buzon, t_paquete* paquete);
void buzon_despachar_mensaje_de(t_buzon* buzon, t_cola* cola);
int buzon_vaciar_hasta_tener(t_buzon* buzon, int espacio);

void buzon_registrar_suscriptor(t_buzon* buzon, t_suscriptor* suscriptor);

void buzon_recibir_ack(t_buzon* buzon, t_ack* ack);

void buzon_imprimir_estado_en(t_buzon* buzon, char* path_archivo);

#endif /* BUZON_H_ */
