/*
 * main.c
 *
 *  Created on: Apr 8, 2020
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <delibird-commons/utils/paquete.h>
#include <delibird-commons/utils/ipc.h>
#include "test/serialization.h"

int main(int argc, char **argv) {

	test_serializarEnviarTodos("127.0.0.1", "8081");

	return EXIT_SUCCESS;
}
