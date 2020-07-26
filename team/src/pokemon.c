//
// Created by utnso on 12/05/20.
//

#include "pokemon.h"

extern t_log* default_logger;

static t_dictionary* pokemon_localizados;

static t_pokemon_mapeado* new_pokemon(char* nombre, int cantidad, t_coord* posicion);
static void liberar_lista_mapeada(void*);
static void liberar_pokemon_mapeado(void*);
static t_pokemon_mapeado* add_pokemon_existente(t_pokemon_mapeado* this_pokemon);

void init_pokemon_map() {
    pokemon_localizados = dictionary_create();
    pthread_mutex_init(&mx_pokemon, NULL);
}

static t_pokemon_mapeado* new_pokemon(char* nombre, int cantidad, t_coord* posicion) {
    t_pokemon_mapeado* this_pokemon = malloc(sizeof(t_pokemon_mapeado));
    this_pokemon->pokemon = string_duplicate(nombre);
    this_pokemon->cantidad = cantidad;
    this_pokemon->ubicacion = malloc(sizeof(t_coord));
    memcpy(this_pokemon->ubicacion, posicion, sizeof(t_coord));
    return this_pokemon;
}

t_list* pokemon_filtrar_especies_capturables(t_list* objetivos) {
    t_list* pokemon_capturables = list_create();

    void _check_and_add(char* obj) {
        t_list* disponibles = pokemon_get_disponibles(obj);
        if (disponibles) {
            list_add(pokemon_capturables, obj);
            list_destroy(disponibles);
        }
    }
    list_iterate(objetivos, (void*)_check_and_add);

    return pokemon_capturables;
}

t_list* pokemon_get_disponibles(char* especie) {
    t_list* ubicaciones_conocidas = dictionary_get(pokemon_localizados, especie);
    if (!ubicaciones_conocidas) return NULL;

    bool _is_available(t_pokemon_mapeado* pokemon) {
        return pokemon->cantidad > 0;
    }
    t_list* disponibles = list_filter(ubicaciones_conocidas, (void*)_is_available);

    return list_size(disponibles)>0 ? disponibles : (list_destroy(disponibles), NULL);
}

t_pokemon_mapeado* pokemon_agregar_al_mapa(char* nombre, int cantidad, t_coord* posicion) {
    t_pokemon_mapeado* this_pokemon = new_pokemon(nombre, cantidad, posicion);

    pthread_mutex_lock(&mx_pokemon);
    if (!dictionary_has_key(pokemon_localizados, nombre)) {
        t_list* list = list_create();
        list_add(list, this_pokemon);
        dictionary_put(pokemon_localizados, nombre, list);
        log_debug(default_logger, "Se agrego un nuevo pokemon (%s) al mapa en la posicion (%d, %d)", nombre, posicion->x, posicion->y);
    } else {
        this_pokemon = add_pokemon_existente(this_pokemon);
    }
    pthread_mutex_unlock(&mx_pokemon);

    return this_pokemon;
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
    pthread_mutex_lock(&mx_pokemon);
    bool b = dictionary_has_key(pokemon_localizados, nombre);
    pthread_mutex_unlock(&mx_pokemon);
    return b;
}

static t_pokemon_mapeado* add_pokemon_existente(t_pokemon_mapeado* this_pokemon) {
    t_list* localizados = (t_list*) dictionary_get(pokemon_localizados, this_pokemon->pokemon);

    bool found = false;
    int cantidad = 0;
    for (int i=0 ; i<list_size(localizados) ; i++) {
        t_pokemon_mapeado* p = (t_pokemon_mapeado*) list_get(localizados, i);
        if (p->ubicacion->x == this_pokemon->ubicacion->x && p->ubicacion->y == this_pokemon->ubicacion->y) {
            found = true;
            p->cantidad++;
            cantidad = p->cantidad;
            liberar_pokemon_mapeado(this_pokemon);
            this_pokemon = p;
            break;
        }
    }

    if (!found) {
        list_add(localizados, this_pokemon);
        cantidad = this_pokemon->cantidad;
    }
    log_debug(default_logger, "Se agrego un %s en (%d, %d). Ahora hay un total de %d en esa posicion",
        this_pokemon->pokemon, this_pokemon->ubicacion->x, this_pokemon->ubicacion->y, cantidad);

    return this_pokemon;
}
