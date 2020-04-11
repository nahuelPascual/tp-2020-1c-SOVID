/*
 * main.c
 *
 *  Created on: Apr 8, 2020
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <delibird-commons/utils/print.h>

int main(int argc, char **argv) {

	char * proceso = string_new();
	
	string_append(&proceso, "GameCard");
	
	print_process(proceso);

	return 0;
}

