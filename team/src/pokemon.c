//
// Created by utnso on 12/05/20.
//

#include "pokemon.h"

t_dictionary* pokemon_localizados;

static t_pokemon* new_pokemon(char* nombre, int cantidad, t_coord* posicion);
static void liberar_lista_mapeada(char* k, void* l);
static void add_pokemon_existente(t_pokemon* this_pokemon);

void init_pokemon_map() {
    pokemon_localizados = dictionary_create();
}

static t_pokemon* new_pokemon(char* nombre, int cantidad, t_coord* posicion) {
    t_pokemon* this_pokemon = malloc(sizeof(t_pokemon));
    this_pokemon->pokemon = nombre;
    this_pokemon->cantidad = cantidad;
    this_pokemon->ubicacion = posicion;
    return this_pokemon;
}

void pokemon_agregar_al_mapa(char* nombre, int cantidad, t_coord* posicion) {
    string_to_upper(nombre);
    t_pokemon* this_pokemon = new_pokemon(nombre, cantidad, posicion);
    if (!is_pokemon_conocido(nombre)) {
        t_list* list = list_create();
        list_add(list, this_pokemon);
        dictionary_put(pokemon_localizados, nombre, list);
    } else {
        add_pokemon_existente(this_pokemon);
    }

}

void pokemon_sacar_del_mapa(char* nombre, t_coord* posicion) {
    t_list* l = dictionary_get(pokemon_localizados, nombre);
    for (int i=0 ; i<list_size(l) ; i++) {
        t_pokemon* p = (t_pokemon*) list_get(l, i);
        if (p->ubicacion->x == posicion->x && p->ubicacion->y == posicion->y) {
            p->cantidad--;
            if (p->cantidad==0) {
                list_remove(l, i);
            }
            break;
        }
    }
}

void pokemon_liberar_mapa() {
    dictionary_iterator(pokemon_localizados, liberar_lista_mapeada);
}

bool is_pokemon_conocido(char* nombre) {
    return dictionary_has_key(pokemon_localizados, nombre);
}

static void liberar_lista_mapeada(char* k, void* l) {
    t_list* list = (t_list*) l;
    list_destroy(list);
}

static void add_pokemon_existente(t_pokemon* this_pokemon) {
    t_list* l = (t_list*) dictionary_get(pokemon_localizados, this_pokemon->pokemon);
    bool found = false;
    for (int i=0 ; i<list_size(l) ; i++) {
        t_pokemon* p = (t_pokemon*) list_get(l, i);
        if (p->ubicacion->x == this_pokemon->ubicacion->x && p->ubicacion->y == this_pokemon->ubicacion->y) {
            found = true;
            p->cantidad++;
            break;
        }
    }
    if (!found) {
        list_add(l, this_pokemon);
    }
}
