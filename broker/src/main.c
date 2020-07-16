/*
 * main.c
 *
 *  Created on: Apr 8, 2020
 *      Author: utnso
 */

#include "mensajeria.h"

int main(int argc, char **argv) {
    mensajeria_inicializar();

    signal(SIGUSR1, (void*) mensajeria_gestionar_signal);

    mensajeria_despachar_mensajes();

    mensajeria_gestionar_clientes();

    return EXIT_SUCCESS;
}
