/*
 * main.c
 *
 *  Created on: Apr 8, 2020
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>

int main(int argc, char **argv) {

    char * proceso = string_new();

    string_append(&proceso, "GameCard");

    puts(proceso);

    return 0;
}
