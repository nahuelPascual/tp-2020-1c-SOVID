//
// Created by utnso on 21/04/20.
//

#include "paquete.h"

t_header* crear_header(uint32_t tipo_mensaje, uint32_t size);

void paquete_liberar(t_paquete* paquete) {
    free(paquete->header);
    free(paquete->payload);
    free(paquete);
}

t_paquete* paquete_from_new_pokemon(t_new_pokemon* pokemon){
    t_paquete* paquete = malloc(sizeof(t_paquete));

    int size = pokemon->nombre_len + sizeof(t_coord) + sizeof(uint32_t)*2;

    void* stream = malloc(size);
    int offset = 0;

    memcpy(stream+offset, &(pokemon->nombre_len), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(stream+offset, pokemon->nombre, pokemon->nombre_len);
    offset += pokemon->nombre_len;
    memcpy(stream+offset, &(pokemon->posicion), sizeof(t_coord));
    offset += sizeof(t_coord);
    memcpy(stream+offset, &(pokemon->cantidad), sizeof(uint32_t));

    paquete->header = crear_header(NEW_POKEMON, size);
    paquete->payload = stream;

    return paquete;
}

t_paquete* paquete_from_localized_pokemon(t_localized_pokemon* pokemon){
    t_paquete* paquete = malloc(sizeof(t_paquete));

    int size = pokemon->nombre_len + pokemon->posiciones_len * sizeof(t_coord) + sizeof(uint32_t) * 2;

    void* stream = malloc(size);
    int offset = 0;

    memcpy(stream+offset, &(pokemon->nombre_len), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(stream+offset, pokemon->nombre, pokemon->nombre_len);
    offset += pokemon->nombre_len;
    memcpy(stream+offset, &(pokemon->posiciones_len), sizeof(uint32_t));
    offset += sizeof(uint32_t);

    for(int i=0; i < pokemon->posiciones_len; i++){
        t_coord* coordenada = (t_coord*) list_get(pokemon->posiciones, i);
        memcpy(stream+offset, coordenada, sizeof(t_coord));
        offset += sizeof(t_coord);
    }

    paquete->header = crear_header(LOCALIZED_POKEMON, size);
    paquete->payload = stream;

    return paquete;
}

t_paquete* paquete_from_get_pokemon(t_get_pokemon* pokemon){
    t_paquete* paquete = malloc(sizeof(t_paquete));

    int size = pokemon->nombre_len + sizeof(uint32_t);

    void* stream = malloc(size);
    int offset = 0;

    memcpy(stream+offset, &(pokemon->nombre_len), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(stream+offset, pokemon->nombre, pokemon->nombre_len);

    paquete->header = crear_header(GET_POKEMON, size);
    paquete->payload = stream;

    return paquete;
}

t_paquete* paquete_from_appeared_pokemon(t_appeared_pokemon* pokemon){
    t_paquete* paquete = malloc(sizeof(t_paquete));

    int size = pokemon->nombre_len + sizeof(t_coord) + sizeof(uint32_t);

    void* stream = malloc(size);
    int offset = 0;

    memcpy(stream+offset, &(pokemon->nombre_len), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(stream+offset, pokemon->nombre, pokemon->nombre_len);
    offset += pokemon->nombre_len;
    memcpy(stream+offset, pokemon->posicion, sizeof(t_coord));
    offset += sizeof(t_coord);


    paquete->header = crear_header(APPEARED_POKEMON, size);
    paquete->payload = stream;

    return paquete;
}

t_paquete* paquete_from_catch_pokemon(t_catch_pokemon* pokemon){
    t_paquete* paquete = malloc(sizeof(t_paquete));

    int size = pokemon->nombre_len + sizeof(t_coord) + sizeof(uint32_t);

    void* stream = malloc(size);
    int offset = 0;

    memcpy(stream+offset, &(pokemon->nombre_len), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(stream+offset, pokemon->nombre, pokemon->nombre_len);
    offset += pokemon->nombre_len;
    memcpy(stream+offset, pokemon->posicion, sizeof(t_coord));
    offset += sizeof(t_coord);


    paquete->header = crear_header(CATCH_POKEMON, size);
    paquete->payload = stream;

    return paquete;
}

t_paquete* paquete_from_caught_pokemon(t_caught_pokemon* pokemon){
    t_paquete* paquete = malloc(sizeof(t_paquete));

    int size = sizeof(uint32_t);

    void* stream = malloc(size);

    memcpy(stream, &(pokemon->is_caught), sizeof(uint32_t));

    paquete->header = crear_header(CAUGHT_POKEMON, size);
    paquete->payload = stream;

    return paquete;
}

t_header* crear_header(uint32_t tipo_mensaje, uint32_t size){
    t_header* header = malloc(sizeof(t_header));

    header->tipo_mensaje = tipo_mensaje;
    header->payload_size = size;

    return header;
}

