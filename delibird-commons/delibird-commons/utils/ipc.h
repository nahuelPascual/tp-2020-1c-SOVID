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
#include <unistd.h>

#include "paquete.h"

int ipc_escuchar_en(char* ip, char* puerto);
int ipc_esperar_cliente(int socket_servidor);
bool ipc_hay_datos_para_recibir_de(int socket);
t_paquete* ipc_recibir_de(int socket);

void ipc_enviar_a(int socket, t_paquete* paquete);
int ipc_conectarse_a(char *ip, char* puerto);

void ipc_cerrar(int socket);

#endif //COMUNICATION_H_
