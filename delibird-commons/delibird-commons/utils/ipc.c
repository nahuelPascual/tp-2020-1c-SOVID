//
// Created by utnso on 10/04/20.
//

#include "ipc.h"

int ipc_escuchar_en(char* ip, char* puerto) {

	int socket_servidor;

	struct addrinfo hints, *servinfo, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &servinfo);

	for(p = servinfo; p != NULL; p = p->ai_next) {

		if ((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
			continue;

		if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
			close(socket_servidor);
			continue;
		}

		break;
	}

	listen(socket_servidor, SOMAXCONN);

	freeaddrinfo(servinfo);

	return socket_servidor;
}

int ipc_esperar_cliente(int socket_servidor) {

	struct sockaddr_in dir_cliente;

	socklen_t tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);

	return socket_cliente;
}

bool ipc_hay_datos_para_recibir_de(int socket) {

	int buffer;

	return recv(socket, &buffer, sizeof(int), MSG_PEEK) > 0;
}

t_paquete* ipc_recibir_de(int socket) {

	t_paquete* paquete = malloc(sizeof(t_paquete));
	t_header* header = malloc(sizeof(t_header));

	recv(socket, header, sizeof(t_header), MSG_WAITALL);
	paquete->payload = malloc(header->payload_size);

	recv(socket, paquete->payload, header->payload_size, MSG_WAITALL);
	paquete->header = header;

	return paquete;
}

void ipc_cerrar(int socket) {

 	close(socket);
}
