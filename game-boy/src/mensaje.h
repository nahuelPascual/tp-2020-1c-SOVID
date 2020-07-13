//
// Created by utnso on 19/04/20.
//

#ifndef MENSAJE_H
#define MENSAJE_H

#include <stdlib.h>
#include <commons/string.h>
#include <delibird-commons/utils/paquete.h>
#include <delibird-commons/model/mensaje.h>
#include "test/serialization.h"

t_paquete* resolver_mensaje(char* proceso, int codOperacion, char** args);
int normalizar_tipo_mensaje(char* nombreMensaje);

#endif //MENSAJE_H
