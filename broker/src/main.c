/*
 * main.c
 *
 *  Created on: Apr 8, 2020
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include "test/deserialization.h"

int main(int argc, char **argv) {

    test_deserializarRecibirTodos("127.0.0.1", "8081");

    return EXIT_SUCCESS;
}
