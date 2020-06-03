//
// Created by utnso on 12/05/20.
//

#ifndef TEAM_POKEMON_H
#define TEAM_POKEMON_H

#include <stdbool.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <delibird-commons/model/mensaje.h>

t_dictionary* pokemon_localizados;

typedef struct {
    char* pokemon;
    t_coord* ubicacion;
    int cantidad;
} t_pokemon_mapeado;

typedef struct {
    char* nombre;
    bool fue_capturado;
} t_pokemon_objetivo;

typedef struct {
    char* nombre;
    bool es_objetivo;
} t_pokemon_capturado;

void init_pokemon_map();
void pokemon_agregar_al_mapa(char* nombre, int cantidad, t_coord* posicion);
void pokemon_sacar_del_mapa(char* nombre, t_coord* posicion);
void pokemon_liberar_mapa();
bool is_pokemon_conocido(char* nombre);

#endif //TEAM_POKEMON_H
