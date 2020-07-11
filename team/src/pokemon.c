//
// Created by utnso on 12/05/20.
//

#include "pokemon.h"

extern t_log* default_logger;

static t_dictionary* pokemon_localizados;
static pthread_mutex_t mx_pokemon;

static t_pokemon_mapeado* new_pokemon(char* nombre, int cantidad, t_coord* posicion);
static void liberar_lista_mapeada(void*);
static void liberar_pokemon_mapeado(void*);
static void add_pokemon_existente(t_pokemon_mapeado* this_pokemon);

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

t_list* pokemon_filtrar_especies_encontradas(t_list* objetivos) {
    bool _especie_conocida(void* obj) {
        char* especie = (char*) obj;
        return dictionary_has_key(pokemon_localizados, especie);
    }
    return list_filter(objetivos, (void*)_especie_conocida);
}

t_list* pokemon_get(char* especie) {
    return (t_list*) dictionary_get(pokemon_localizados, especie);
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
        add_pokemon_existente(this_pokemon);
    }
    pthread_mutex_unlock(&mx_pokemon);

    return this_pokemon;
}

void pokemon_sacar_del_mapa(char* nombre, t_coord* posicion) {
    pthread_mutex_lock(&mx_pokemon);
    t_list* ubicaciones_pokemon = dictionary_get(pokemon_localizados, nombre);
    for (int i=0 ; i<list_size(ubicaciones_pokemon) ; i++) {
        t_pokemon_mapeado* p = (t_pokemon_mapeado*) list_get(ubicaciones_pokemon, i);
        if (p->ubicacion->x == posicion->x && p->ubicacion->y == posicion->y) {
            p->cantidad--;
            if (p->cantidad==0) {
                if (list_size(ubicaciones_pokemon) == 1) dictionary_remove(pokemon_localizados, nombre); // si es la unica ubicacion que queda, se borra pokemon del mapa
                list_destroy_and_destroy_elements(ubicaciones_pokemon, (void*)liberar_pokemon_mapeado);
            }
            break;
        }
    }
    pthread_mutex_unlock(&mx_pokemon);
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

static void add_pokemon_existente(t_pokemon_mapeado* this_pokemon) {
    t_list* l = (t_list*) dictionary_get(pokemon_localizados, this_pokemon->pokemon);
    bool found = false;
    int cantidad = 0;
    for (int i=0 ; i<list_size(l) ; i++) {
        t_pokemon_mapeado* p = (t_pokemon_mapeado*) list_get(l, i);
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
        list_add(l, this_pokemon);
        cantidad = this_pokemon->cantidad;
    }
    log_debug(default_logger, "Se agrego un %s en (%d, %d). Ahora hay un total de %d en esa posicion",
        this_pokemon->pokemon, this_pokemon->ubicacion->x, this_pokemon->ubicacion->y, cantidad);
}
