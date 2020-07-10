//
// Created by utnso on 4/07/20.
//

#include "deadlock.h"

static t_intercambio* new_intercambio(t_pokemon_objetivo* entrego, t_pokemon_objetivo* recibo, t_entrenador* otro_entrenador);
static t_pokemon_objetivo* encontrar_pokemon_para_intercambio(t_entrenador* entrenador, t_entrenador* otro_entrenador);
static t_entrenador* detectar_simple(t_entrenador*);
static t_entrenador* detectar_encadenado(t_entrenador*, int, t_pokemon_objetivo**);

t_entrenador* deadlock_detectar(t_entrenador* entrenador) {
    if (entrenador->deadlock) {
        // cuando vuelvo de resolver un deadlock entre Entrenador1 y Entrenador2,
        // puede darse que Entrenador1 se planifique para resolver un nuevo deadlock con Entrenador2
        // y luego Entrenador2 quiera planificarse para resolver deadlock con Entrenador3.
        return NULL;
    }

    t_entrenador* otro_entrenador = detectar_simple(entrenador);

    if (!otro_entrenador) {
        t_pokemon_objetivo* entrega_entrenador = NULL;
        otro_entrenador = detectar_encadenado(entrenador, entrenador->id, &entrega_entrenador);
    }

    return otro_entrenador;
}

static t_entrenador* detectar_encadenado(t_entrenador* e, int id_primer_entrenador, t_pokemon_objetivo** entrega_primer_entrenador) {
    t_list* bloqueados = entrenador_get_bloqueados(e->id);

    t_entrenador* otro_entrenador = NULL;
    t_pokemon_objetivo* recibo = NULL;
    for (int i = 0 ; i<list_size(bloqueados) ; i++) { //TODO se puede mejorar pasando solo los entrenadores posibles (ninguno de los ya seleccionados) -> cuidado con leaks
        t_entrenador* un_bloqueado = list_get(bloqueados, i);
        
        t_pokemon_objetivo* pokemon = encontrar_pokemon_para_intercambio(e, un_bloqueado);
        if (!pokemon) continue;
        
        if (un_bloqueado->id == id_primer_entrenador || detectar_encadenado(un_bloqueado, id_primer_entrenador, entrega_primer_entrenador)) {
            if (*entrega_primer_entrenador==NULL) *entrega_primer_entrenador = pokemon;
            otro_entrenador = un_bloqueado;
            recibo = pokemon;
            break;
        }
        else continue;
    }

    if (e->id == id_primer_entrenador && otro_entrenador != NULL) {
        e->intercambio = new_intercambio(*entrega_primer_entrenador, recibo, otro_entrenador);
    }

    return otro_entrenador;
}

static t_entrenador* detectar_simple(t_entrenador* entrenador) {
    t_list* bloqueados = entrenador_get_bloqueados(entrenador->id);

    t_entrenador* otro_entrenador = NULL;
    for (int i = 0 ; i < list_size(bloqueados) ; i++) {
        t_entrenador* un_bloqueado = list_get(bloqueados, i);
        t_pokemon_objetivo* entrego = encontrar_pokemon_para_intercambio(un_bloqueado, entrenador);
        t_pokemon_objetivo* recibo = encontrar_pokemon_para_intercambio(entrenador, un_bloqueado);
        if (entrego != NULL && recibo != NULL) {
            entrenador->intercambio = new_intercambio(entrego, recibo, un_bloqueado);
            otro_entrenador = un_bloqueado;
        }
    }

    free(bloqueados);

    return otro_entrenador;
}

/**
 * devuelve el pokemon que le falta a "entrenador" y que a "otro_entrenador" le sobra
 */ 
static t_pokemon_objetivo* encontrar_pokemon_para_intercambio(t_entrenador* entrenador, t_entrenador* otro_entrenador) {
    t_list* mis_faltantes = entrenador_calcular_pokemon_faltantes(entrenador);
    bool _es_mi_faltante(void* item) {
        t_pokemon_objetivo* sobrante = (t_pokemon_objetivo*) item;
        bool _equals(void* item) {
            t_pokemon_objetivo* faltante = (t_pokemon_objetivo*) item;
            return string_equals_ignore_case(faltante->nombre, sobrante->nombre);
        }
        return list_any_satisfy(mis_faltantes, (void*)_equals);
    }

    t_list* sobrantes_otro = entrenador_calcular_pokemon_sobrantes(otro_entrenador);
    t_pokemon_objetivo* mi_faltante = list_find(sobrantes_otro, (void*)_es_mi_faltante);

    free(mis_faltantes);
    free(sobrantes_otro);

    return mi_faltante;
}

static t_intercambio* new_intercambio(t_pokemon_objetivo* entrego, t_pokemon_objetivo* recibo, t_entrenador* otro_entrenador) {
    t_intercambio* intercambio = malloc(sizeof(t_intercambio));
    intercambio->entrego_pokemon = entrego->nombre;
    intercambio->recibo_pokemon = recibo->nombre;
    intercambio->id_otro_entrenador = otro_entrenador->id;
    intercambio->ubicacion = otro_entrenador->posicion;
    intercambio->remaining_intercambio = 5;
    return intercambio;
}
