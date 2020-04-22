//
// Created by utnso on 21/04/20.
//

#ifndef PAQUETE_H
#define PAQUETE_H

#include <stdint.h>

typedef struct {
    uint32_t tipo_mensaje;
    uint32_t payload_size;
} t_header;

typedef struct {
    t_header* header;
    void* payload;
} t_paquete;

void paquete_liberar(t_paquete* paquete);

#endif //PAQUETE_H
