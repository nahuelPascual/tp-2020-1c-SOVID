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

t_new_pokemon* paquete_to_new_pokemon(t_paquete* paquete) {
    t_new_pokemon* mensaje = malloc(sizeof(t_new_pokemon));

    int offset = 0;
    memcpy(&mensaje->nombre_len, paquete->payload + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    mensaje->nombre = malloc(mensaje->nombre_len);
    memcpy(mensaje->nombre, paquete->payload + offset, mensaje->nombre_len);
    offset += mensaje->nombre_len;
    mensaje->posicion = malloc(sizeof(t_coord));
    memcpy(mensaje->posicion, paquete->payload + offset, sizeof(t_coord));
    offset += sizeof(t_coord);
    memcpy(&mensaje->cantidad, paquete->payload + offset, sizeof(uint32_t));

    return mensaje;
}

t_localized_pokemon* paquete_to_localized_pokemon(t_paquete* paquete) {
    t_localized_pokemon* mensaje = malloc(sizeof(t_localized_pokemon));

    int offset = 0;
    memcpy(&mensaje->nombre_len, paquete->payload + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    mensaje->nombre = malloc(mensaje->nombre_len);
    memcpy(mensaje->nombre, paquete->payload + offset, mensaje->nombre_len);
    offset += mensaje->nombre_len;
    memcpy(&mensaje->posiciones_len, paquete->payload + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    mensaje->posiciones = list_create();
    for(int i = 0; i < mensaje->posiciones_len; i++) {
        t_coord* posicion = malloc(sizeof(t_coord));
        memcpy(posicion, paquete->payload + offset, sizeof(t_coord));
        offset += sizeof(t_coord);

        list_add(mensaje->posiciones, posicion);
    }

    return mensaje;
}

t_get_pokemon* paquete_to_get_pokemon(t_paquete* paquete) {
    t_get_pokemon* mensaje = malloc(sizeof(t_get_pokemon));

    int offset = 0;
    memcpy(&mensaje->nombre_len, paquete->payload + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    mensaje->nombre = malloc(mensaje->nombre_len);
    memcpy(mensaje->nombre, paquete->payload + offset, mensaje->nombre_len);

    return mensaje;
}

t_appeared_pokemon* paquete_to_appeared_pokemon(t_paquete* paquete) {
    t_appeared_pokemon* mensaje = malloc(sizeof(t_appeared_pokemon));

    int offset = 0;
    memcpy(&mensaje->nombre_len, paquete->payload + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    mensaje->nombre = malloc(mensaje->nombre_len);
    memcpy(mensaje->nombre, paquete->payload + offset, mensaje->nombre_len);
    offset += mensaje->nombre_len;
    mensaje->posicion = malloc(sizeof(t_coord));
    memcpy(mensaje->posicion, paquete->payload + offset, sizeof(t_coord));

    return mensaje;
}

t_catch_pokemon* paquete_to_catch_pokemon(t_paquete* paquete) {
    t_catch_pokemon* mensaje = malloc(sizeof(t_catch_pokemon));

    int offset = 0;
    memcpy(&mensaje->nombre_len, paquete->payload + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    mensaje->nombre = malloc(mensaje->nombre_len);
    memcpy(mensaje->nombre, paquete->payload + offset, mensaje->nombre_len);
    offset += mensaje->nombre_len;
    mensaje->posicion = malloc(sizeof(t_coord));
    memcpy(mensaje->posicion, paquete->payload + offset, sizeof(t_coord));

    return mensaje;
}

t_caught_pokemon* paquete_to_caught_pokemon(t_paquete* paquete) {
    t_caught_pokemon* mensaje = malloc(sizeof(t_caught_pokemon));

    memcpy(&mensaje->is_caught, paquete->payload, sizeof(uint32_t));

    return mensaje;
}

t_paquete* paquete_from_new_pokemon(t_new_pokemon* pokemon) {
    t_paquete* paquete = malloc(sizeof(t_paquete));

    int size = pokemon->nombre_len + sizeof(t_coord) + sizeof(uint32_t) * 2;

    void* stream = malloc(size);
    int offset = 0;

    memcpy(stream + offset, &pokemon->nombre_len, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(stream + offset, pokemon->nombre, pokemon->nombre_len);
    offset += pokemon->nombre_len;
    memcpy(stream + offset, pokemon->posicion, sizeof(t_coord));
    offset += sizeof(t_coord);
    memcpy(stream + offset, &pokemon->cantidad, sizeof(uint32_t));

    paquete->header = crear_header(NEW_POKEMON, size);
    paquete->payload = stream;

    return paquete;
}

t_paquete* paquete_from_localized_pokemon(t_localized_pokemon* pokemon) {
    t_paquete* paquete = malloc(sizeof(t_paquete));

    int size = pokemon->nombre_len + pokemon->posiciones_len * sizeof(t_coord) + sizeof(uint32_t) * 2;

    void* stream = malloc(size);
    int offset = 0;

    memcpy(stream + offset, &pokemon->nombre_len, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(stream + offset, pokemon->nombre, pokemon->nombre_len);
    offset += pokemon->nombre_len;
    memcpy(stream + offset, &pokemon->posiciones_len, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    for(int i = 0; i < pokemon->posiciones_len; i++) {
        t_coord* posicion = (t_coord*) list_get(pokemon->posiciones, i);
        memcpy(stream + offset, posicion, sizeof(t_coord));
        offset += sizeof(t_coord);
    }

    paquete->header = crear_header(LOCALIZED_POKEMON, size);
    paquete->payload = stream;

    return paquete;
}

t_paquete* paquete_from_get_pokemon(t_get_pokemon* pokemon) {
    t_paquete* paquete = malloc(sizeof(t_paquete));

    int size = pokemon->nombre_len + sizeof(uint32_t);

    void* stream = malloc(size);
    int offset = 0;

    memcpy(stream + offset, &pokemon->nombre_len, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(stream + offset, pokemon->nombre, pokemon->nombre_len);

    paquete->header = crear_header(GET_POKEMON, size);
    paquete->payload = stream;

    return paquete;
}

t_paquete* paquete_from_appeared_pokemon(t_appeared_pokemon* pokemon) {
    t_paquete* paquete = malloc(sizeof(t_paquete));

    int size = pokemon->nombre_len + sizeof(t_coord) + sizeof(uint32_t);

    void* stream = malloc(size);
    int offset = 0;

    memcpy(stream + offset, &pokemon->nombre_len, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(stream + offset, pokemon->nombre, pokemon->nombre_len);
    offset += pokemon->nombre_len;
    memcpy(stream + offset, pokemon->posicion, sizeof(t_coord));
    offset += sizeof(t_coord);

    paquete->header = crear_header(APPEARED_POKEMON, size);
    paquete->payload = stream;

    return paquete;
}

t_paquete* paquete_from_catch_pokemon(t_catch_pokemon* pokemon) {
    t_paquete* paquete = malloc(sizeof(t_paquete));

    int size = pokemon->nombre_len + sizeof(t_coord) + sizeof(uint32_t);

    void* stream = malloc(size);
    int offset = 0;

    memcpy(stream + offset, &pokemon->nombre_len, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(stream + offset, pokemon->nombre, pokemon->nombre_len);
    offset += pokemon->nombre_len;
    memcpy(stream + offset, pokemon->posicion, sizeof(t_coord));
    offset += sizeof(t_coord);

    paquete->header = crear_header(CATCH_POKEMON, size);
    paquete->payload = stream;

    return paquete;
}

t_paquete* paquete_from_caught_pokemon(t_caught_pokemon* pokemon) {
    t_paquete* paquete = malloc(sizeof(t_paquete));

    int size = sizeof(uint32_t);

    void* stream = malloc(size);

    memcpy(stream, &pokemon->is_caught, sizeof(uint32_t));

    paquete->header = crear_header(CAUGHT_POKEMON, size);
    paquete->payload = stream;

    return paquete;
}

t_header* crear_header(uint32_t tipo_mensaje, uint32_t size) {
    t_header* header = malloc(sizeof(t_header));

    header->tipo_mensaje = tipo_mensaje;
    header->payload_size = size;

    return header;
}
