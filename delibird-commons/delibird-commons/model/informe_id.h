/*
 * informe_id.h
 *
 *  Created on: 8 may. 2020
 *      Author: utnso
 */

#ifndef MODEL_INFORME_ID_H_
#define MODEL_INFORME_ID_H_

#include <stdlib.h>
#include <stdint.h>

typedef struct {
    uint32_t id_mensaje;
} t_informe_id;

t_informe_id* informe_id_crear(uint32_t id_mensaje);

void informe_id_liberar(t_informe_id* informe_id);

#endif /* MODEL_INFORME_ID_H_ */
