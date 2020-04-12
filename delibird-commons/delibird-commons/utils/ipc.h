//
// Created by utnso on 10/04/20.
//

#ifndef COMUNICATION_H_
#define COMUNICATION_H_

#include <stdint.h>

typedef struct {
    uint32_t size;
    void* stream;
} t_buffer;

typedef struct {
    uint32_t operacion;
    t_buffer* buffer;
} t_paquete;

#endif //COMUNICATION_H_
