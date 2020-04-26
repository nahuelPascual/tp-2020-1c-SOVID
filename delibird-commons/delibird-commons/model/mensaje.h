//
// Created by utnso on 9/04/20.
//

#ifndef MENSAJE_H_
#define MENSAJE_H_

#include <commons/collections/list.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

enum {
    STRING,
    NEW_POKEMON,
    LOCALIZED_POKEMON,
    GET_POKEMON,
    APPEARED_POKEMON,
    CATCH_POKEMON,
    CAUGHT_POKEMON
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

typedef struct{
    uint32_t is_caught;
} t_caught_pokemon;

t_new_pokemon* mensaje_crear_new_pokemon(char* nombre, uint32_t x, uint32_t y, uint32_t cantidad);
t_localized_pokemon* mensaje_crear_localized_pokemon(char* nombre, uint32_t cantidadPosiciones, ...);
t_get_pokemon* mensaje_crear_get_pokemon(char* nombre);
t_appeared_pokemon* mensaje_crear_appeared_pokemon(char* nombre, uint32_t x, uint32_t y);
t_catch_pokemon* mensaje_crear_catch_pokemon(char* nombre, uint32_t x, uint32_t y);
t_caught_pokemon* mensaje_crear_caught_pokemon(uint32_t booleano);

void mensaje_liberar_new_pokemon(t_new_pokemon*);
void mensaje_liberar_localized_pokemon(t_localized_pokemon*);
void mensaje_liberar_get_pokemon(t_get_pokemon*);
void mensaje_liberar_appeared_pokemon(t_appeared_pokemon*);
void mensaje_liberar_catch_pokemon(t_catch_pokemon*);
void mensaje_liberar_caught_pokemon(t_caught_pokemon*);

#endif //MENSAJE_H_
