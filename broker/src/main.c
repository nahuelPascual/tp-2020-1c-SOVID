/*
 * main.c
 *
 *  Created on: Apr 8, 2020
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <delibird-commons/utils/ipc.h>

int main(int argc, char **argv) {

	int broker = ipc_escuchar_en("127.0.0.1", "8081");

	int gameBoy = ipc_esperar_cliente(broker);

	while(ipc_hay_datos_para_recibir_de(gameBoy)) {

		t_paquete* paquete = ipc_recibir_de(gameBoy);

		//Hacer algo con el paquete

	}
	
	ipc_cerrar(gameBoy);

	return EXIT_SUCCESS;
}

