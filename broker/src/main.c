/*
 * main.c
 *
 *  Created on: Apr 8, 2020
 *      Author: utnso
 */

#include "mensajeria.h"

int main(int argc, char **argv) {
    pthread_t gestor_de_clientes;
    pthread_t despachantes[6];

    mensajeria_inicializar();

    pthread_create(&gestor_de_clientes, NULL, (void*) mensajeria_gestionar_clientes, NULL);

    for(t_tipo_mensaje tipo_mensaje = 0; tipo_mensaje < 6; tipo_mensaje++) {
        pthread_create(&despachantes[tipo_mensaje], NULL, (void*) mensajeria_despachar_mensajes_de, (void*) tipo_mensaje);
    }

    pthread_join(gestor_de_clientes, NULL);
    for(t_tipo_mensaje tipo_mensaje = 0; tipo_mensaje < 6; tipo_mensaje++) {
        pthread_join(despachantes[tipo_mensaje], NULL);
    }

    return EXIT_SUCCESS;
}
