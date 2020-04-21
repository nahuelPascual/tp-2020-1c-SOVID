//
// Created by utnso on 21/04/20.
//

#include "paquete.h"

void paquete_liberar(t_paquete* paquete) {
    free(paquete->header);
    free(paquete->payload);
    free(paquete);
}