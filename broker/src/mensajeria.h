/*
 * mensajeria.h
 *
 *  Created on: 17 may. 2020
 *      Author: utnso
 */

#ifndef MENSAJERIA_H_
#define MENSAJERIA_H_

#include <pthread.h>

#include <commons/string.h>
#include <commons/collections/dictionary.h>
#include <delibird-commons/utils/ipc.h>

#include "cola.h"

t_dictionary* diccionario_de_colas;
t_list* mensajes_pendientes_de_ack;
int id_mensaje;

pthread_mutex_t mutex_pendientes_de_ack;
pthread_mutex_t mutex_id_mensaje;

void mensajeria_inicializar();
void mensajeria_gestionar_clientes();
void mensajeria_despachar_mensajes_de(t_tipo_mensaje tipo_mensaje);

#endif /* MENSAJERIA_H_ */
