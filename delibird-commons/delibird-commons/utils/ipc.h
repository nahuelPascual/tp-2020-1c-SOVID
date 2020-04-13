//
// Created by utnso on 10/04/20.
//

#ifndef COMUNICATION_H_
#define COMUNICATION_H_

#include <stdint.h>

typedef struct {
    uint32_t cod_op;
    uint32_t size;
} t_header;

typedef struct {
    t_header* header;
    void* payload;
} t_paquete;

#endif //COMUNICATION_H_
