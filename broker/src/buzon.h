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
} t_buzon;

t_buzon* buzon_crear();

bool buzon_almacenar_mensaje(t_buzon* buzon, t_paquete* paquete);
void buzon_despachar_mensaje_de(t_buzon* buzon, t_cola* cola);

void buzon_registrar_suscriptor(t_buzon* buzon, t_suscriptor* suscriptor);

void buzon_recibir_ack(t_buzon* buzon, t_ack* ack);

void buzon_informar_id_mensaje_a(int id_mensaje, int socket_suscriptor);

#endif /* BUZON_H_ */
