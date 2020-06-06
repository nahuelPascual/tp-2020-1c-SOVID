//
// Created by utnso on 12/05/20.
//

#include "pokemon.h"

t_dictionary* pokemon_localizados;

static t_pokemon_mapeado* new_pokemon(char* nombre, int cantidad, t_coord* posicion);
static void liberar_lista_mapeada(void*);
static void liberar_pokemon_mapeado(void*);
static void add_pokemon_existente(t_pokemon_mapeado* this_pokemon);

void init_pokemon_map() {
    pokemon_localizados = dictionary_create();
}

static t_pokemon_mapeado* new_pokemon(char* nombre, int cantidad, t_coord* posicion) {
    t_pokemon_mapeado* this_pokemon = malloc(sizeof(t_pokemon_mapeado));
    this_pokemon->pokemon = string_duplicate(nombre);
    this_pokemon->cantidad = cantidad;
    memcpy(this_pokemon->ubicacion, posicion, sizeof(t_coord));
    return this_pokemon;
}

t_pokemon_mapeado* pokemon_agregar_al_mapa(char* nombre, int cantidad, t_coord* posicion) {
    t_pokemon_mapeado* this_pokemon = new_pokemon(nombre, cantidad, posicion);
    if (!is_pokemon_conocido(nombre)) {
        t_list* list = list_create();
        list_add(list, this_pokemon);
        dictionary_put(pokemon_localizados, string_duplicate(nombre), list); // vuelvo a duplicar para no usar como key el string de ningun item de la lista
    } else {
        add_pokemon_existente(this_pokemon);
    }
    return this_pokemon;
}

void pokemon_sacar_del_mapa(char* nombre, t_coord* posicion) {
    t_list* ubicaciones_pokemon = dictionary_get(pokemon_localizados, nombre);
    for (int i=0 ; i<list_size(ubicaciones_pokemon) ; i++) {
        t_pokemon_mapeado* p = (t_pokemon_mapeado*) list_get(ubicaciones_pokemon, i);
        if (p->ubicacion->x == posicion->x && p->ubicacion->y == posicion->y) {
            p->cantidad--;
            if (p->cantidad==0) {
                list_remove_and_destroy_element(ubicaciones_pokemon, i, (void*)liberar_pokemon_mapeado);
            }
            if (list_size(ubicaciones_pokemon) == 0) {
                dictionary_remove_and_destroy(pokemon_localizados, nombre, (void*)liberar_lista_mapeada);
            }
            break;
        }
    }
    //TODO habria que eliminar el pokemon del mapa si eliminamos la unica posicion donde estaba
}

void pokemon_liberar_mapa() {
    dictionary_destroy_and_destroy_elements(pokemon_localizados, (void*)liberar_lista_mapeada);
}

static void liberar_lista_mapeada(void* lista) {
    list_destroy_and_destroy_elements((t_list*)lista, (void*)liberar_pokemon_mapeado);
}

static void liberar_pokemon_mapeado(void* e) {
    t_pokemon_mapeado* pokemon = (t_pokemon_mapeado*) e;
    free(pokemon->pokemon);
    free(pokemon->ubicacion);
    free(pokemon);
}

bool is_pokemon_conocido(char* nombre) {
    return dictionary_has_key(pokemon_localizados, nombre);
}

static void add_pokemon_existente(t_pokemon_mapeado* this_pokemon) {
    t_list* l = (t_list*) dictionary_get(pokemon_localizados, this_pokemon->pokemon);
    bool found = false;
    for (int i=0 ; i<list_size(l) ; i++) {
        t_pokemon_mapeado* p = (t_pokemon_mapeado*) list_get(l, i);
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
