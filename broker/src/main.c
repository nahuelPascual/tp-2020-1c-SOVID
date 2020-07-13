/*
 * main.c
 *
 *  Created on: Apr 8, 2020
 *      Author: utnso
 */

#include "mensajeria.h"

t_log* logger;

int main(int argc, char **argv) {
    logger = log_create("broker.log", "BROKER", true, LOG_LEVEL_INFO); // TODO levantar path de la config
    mensajeria_inicializar();

    mensajeria_despachar_mensajes();

    mensajeria_gestionar_clientes();

    return EXIT_SUCCESS;
}
