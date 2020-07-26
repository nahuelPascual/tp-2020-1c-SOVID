/*
 * informe_id.c
 *
 *  Created on: 8 may. 2020
 *      Author: utnso
 */

#include "informe_id.h"

t_informe_id* informe_id_crear(uint32_t id_mensaje) {
    t_informe_id* informe_id = malloc(sizeof(t_informe_id));

    informe_id->id_mensaje = id_mensaje;

    return informe_id;
}

void informe_id_liberar(t_informe_id* informe_id) {
    free(informe_id);
}
