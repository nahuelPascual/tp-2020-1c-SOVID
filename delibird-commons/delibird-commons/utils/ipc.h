//
// Created by utnso on 10/04/20.
//

#ifndef COMUNICATION_H_
#define COMUNICATION_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include "logger.h"
#include "paquete.h"

int ipc_escuchar_en(char* ip, char* puerto);
int ipc_esperar_cliente(int socket_servidor);
bool ipc_hay_datos_para_recibir_de(int socket);
t_paquete* ipc_recibir_de(int socket);

bool ipc_enviar_a(int socket, t_paquete* paquete);
int ipc_conectarse_a(char *ip, char* puerto);
void ipc_cerrar(int socket);
void ipc_crear_gameboy_listener(t_listener_config*);
int ipc_enviar_broker(t_paquete*, char*, char*);
void ipc_enviar_ack(uint32_t, uint32_t, int);
int ipc_enviar_suscripcion(t_tipo_mensaje, uint32_t, uint32_t, t_listener_config*);
void ipc_suscribirse_a(t_tipo_mensaje, uint32_t, uint32_t, t_listener_config*);


#endif //COMUNICATION_H_
