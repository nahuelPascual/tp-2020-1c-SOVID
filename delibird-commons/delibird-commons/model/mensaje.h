//
// Created by utnso on 9/04/20.
//

#ifndef MENSAJE_H_
#define MENSAJE_H_

#include <commons/collections/list.h>
#include <stdint.h>

enum {
    STRING,
    NEW_POKEMON, APPEARED_POKEMON,
    GET_POKEMON, LOCALIZED_POKEMON,
    CATCH_POKEMON, CAUGHT_POKEMON
};

typedef struct {
    uint32_t x;
    uint32_t y;
} t_coord;

typedef struct {
    uint32_t nombre_len;
    char* nombre;
    t_coord* posicion;
    uint32_t cantidad;
} t_new_pokemon;

typedef struct {
    uint32_t nombre_len;
    char* nombre;
    uint32_t posiciones_len;
    t_list* posiciones;
} t_localized_pokemon;

typedef struct {
    uint32_t nombre_len;
    char* nombre;
} t_get_pokemon;

typedef struct {
    uint32_t nombre_len;
    char* nombre;
    t_coord* posicion;
} t_appeared_pokemon;

typedef struct {
    uint32_t nombre_len;
    char* nombre;
    t_coord* posicion;
} t_catch_pokemon;

typedef uint32_t t_caught_pokemon;

#endif //MENSAJE_H_
