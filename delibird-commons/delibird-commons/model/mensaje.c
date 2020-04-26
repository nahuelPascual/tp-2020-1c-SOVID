//
// Created by utnso on 9/04/20.
//

#include "mensaje.h"

t_new_pokemon* mensaje_crear_new_pokemon(char* nombre, uint32_t x, uint32_t y, uint32_t cantidad) {
    t_new_pokemon* pokemon = malloc(sizeof(t_new_pokemon));

    pokemon->nombre_len = strlen(nombre) + 1;
    pokemon->nombre = malloc(pokemon->nombre_len);
    strcpy(pokemon->nombre, nombre);
    pokemon->posicion = malloc(sizeof(t_coord));
    pokemon->posicion->x = x;
    pokemon->posicion->y = y;
    pokemon->cantidad = cantidad;

    return pokemon;
}

t_localized_pokemon* mensaje_crear_localized_pokemon(char* nombre, uint32_t cantidadPosiciones, ...) {
    va_list coordenadas;
    t_list* posiciones = list_create();

    va_start(coordenadas, cantidadPosiciones*2);
    for (int i=0; i<cantidadPosiciones; ++i) {
        t_coord* posicion = malloc(sizeof(t_coord));
        posicion->x = va_arg(coordenadas, uint32_t);
        posicion->y = va_arg(coordenadas, uint32_t);
        list_add(posiciones, posicion);
    }
    va_end(coordenadas);

    t_localized_pokemon* pokemon = malloc(sizeof(t_localized_pokemon));
    pokemon->nombre_len = strlen(nombre) + 1;
    pokemon->nombre = malloc(pokemon->nombre_len);
    strcpy(pokemon->nombre, nombre);
    pokemon->posiciones_len = cantidadPosiciones;
    pokemon->posiciones = posiciones;

    return pokemon;
}

t_get_pokemon* mensaje_crear_get_pokemon(char* nombre) {
    t_get_pokemon* pokemon = malloc(sizeof(t_get_pokemon));

    pokemon->nombre_len = strlen(nombre) + 1;
    pokemon->nombre = malloc(pokemon->nombre_len);
    strcpy(pokemon->nombre, nombre);

    return pokemon;
}

t_appeared_pokemon* mensaje_crear_appeared_pokemon(char* nombre, uint32_t x, uint32_t y) {
    t_appeared_pokemon* pokemon = malloc(sizeof(t_appeared_pokemon));

    pokemon->nombre_len = strlen(nombre) + 1;
    pokemon->nombre = malloc(pokemon->nombre_len);
    strcpy(pokemon->nombre, nombre);
    pokemon->posicion = malloc(sizeof(t_coord));
    pokemon->posicion->x = x;
    pokemon->posicion->y = y;

    return pokemon;
}

t_catch_pokemon* mensaje_crear_catch_pokemon(char* nombre, uint32_t x, uint32_t y) {
    t_catch_pokemon* pokemon = malloc(sizeof(t_catch_pokemon));

    pokemon->nombre_len = strlen(nombre) + 1;
    pokemon->nombre = malloc(pokemon->nombre_len);
    strcpy(pokemon->nombre, nombre);
    pokemon->posicion = malloc(sizeof(t_coord));
    pokemon->posicion->x = x;
    pokemon->posicion->y = y;

    return pokemon;
}

t_caught_pokemon* mensaje_crear_caught_pokemon(uint32_t booleano) {
    t_caught_pokemon* pokemon = malloc(sizeof(t_caught_pokemon));

    pokemon->is_caught = booleano;

    return pokemon;
}

void mensaje_liberar_new_pokemon(t_new_pokemon* mensaje) {
    free(mensaje->nombre);
    free(mensaje->posicion);
    free(mensaje);
}

void mensaje_liberar_localized_pokemon(t_localized_pokemon* mensaje) {
    free(mensaje->nombre);
    list_destroy_and_destroy_elements(mensaje->posiciones, free);
    free(mensaje);
}

void mensaje_liberar_get_pokemon(t_get_pokemon* mensaje) {
    free(mensaje->nombre);
    free(mensaje);
}

void mensaje_liberar_appeared_pokemon(t_appeared_pokemon* mensaje) {
    free(mensaje->nombre);
    free(mensaje->posicion);
    free(mensaje);
}

void mensaje_liberar_catch_pokemon(t_catch_pokemon* mensaje) {
    free(mensaje->nombre);
    free(mensaje->posicion);
    free(mensaje);
}

void mensaje_liberar_caught_pokemon(t_caught_pokemon* mensaje) {
    free(mensaje);
}