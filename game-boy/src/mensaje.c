//
// Created by utnso on 19/04/20.
//

#include "mensaje.h"

extern t_log* logger;
static const char* GAMECARD = "gamecard";
static const char* BROKER = "broker";

t_paquete* resolver_mensaje(char* proceso, int codOperacion, char** args) {
    void* msg;
    t_paquete* p;

    switch(codOperacion) {
    case NEW_POKEMON:
        msg = mensaje_crear_new_pokemon(args[3], atoi(args[4]), atoi(args[5]), atoi(args[6]));
        p = paquete_from_new_pokemon((t_new_pokemon*) msg);
        if(string_equals_ignore_case(proceso, GAMECARD)) {
            if (args[7] != NULL) {
                p->header->id_mensaje = atoi(args[7]);
            } else {
                log_error(logger, "Mensaje NEW_POKEMON necesita ID_MENSAJE para enviarse a GAME-CARD");
                return NULL;
            }
        }
        mensaje_liberar_new_pokemon(msg);
        return p;
    case GET_POKEMON:
        msg = mensaje_crear_get_pokemon(args[3]);
        p = paquete_from_get_pokemon((t_get_pokemon*) msg);
        if(string_equals_ignore_case(proceso, GAMECARD)) {
            if (args[4] != NULL) {
                p->header->id_mensaje = atoi(args[4]);
            } else {
                log_error(logger, "Mensaje GET_POKEMON necesita ID_MENSAJE para enviarse a GAME-CARD");
                return NULL;
            }
        }
        mensaje_liberar_get_pokemon(msg);
        return p;
    case APPEARED_POKEMON:
        msg = mensaje_crear_appeared_pokemon(args[3], atoi(args[4]), atoi(args[5]));
        p = paquete_from_appeared_pokemon((t_appeared_pokemon*) msg);
        if(string_equals_ignore_case(proceso, BROKER)) {
            if(args[6] != NULL) {
                p->header->correlation_id_mensaje = atoi(args[6]);
            } else {
                log_error(logger, "Mensaje APPEARED_POKEMON necesita CORRELATION_ID para enviarse a BROKER");
                return NULL;
            }
        }
        mensaje_liberar_appeared_pokemon(msg);
        return p;
    case CATCH_POKEMON:
        msg = mensaje_crear_catch_pokemon(args[3], atoi(args[4]), atoi(args[5]));
        p = paquete_from_catch_pokemon((t_catch_pokemon*) msg);
        if(string_equals_ignore_case(proceso, GAMECARD)) {
            if(args[6] != NULL) {
                p->header->id_mensaje = atoi(args[6]);
            } else {
                log_error(logger, "Mensaje CATCH_POKEMON necesita ID_MENSAJE para enviarse a GAME-CARD");
                return NULL;
            }
        }
        mensaje_liberar_catch_pokemon(msg);
        return p;
    case CAUGHT_POKEMON:
        msg = mensaje_crear_caught_pokemon(string_equals_ignore_case(args[4], "OK") ? 1 : 0); // nos mandan OK/FAIL y lo traducimos a 1/0
        p = paquete_from_caught_pokemon((t_caught_pokemon*) msg);
        p->header->correlation_id_mensaje = atoi(args[3]);
        mensaje_liberar_caught_pokemon(msg);
        return p;
    }

    return NULL;
}

int normalizar_tipo_mensaje(char* mensaje) {
    if(string_equals_ignore_case(mensaje, "NEW_POKEMON")) {
        return NEW_POKEMON;
    }
    if(string_equals_ignore_case(mensaje, "LOCALIZED_POKEMON")) {
        return LOCALIZED_POKEMON;
    }
    if(string_equals_ignore_case(mensaje, "GET_POKEMON")) {
        return GET_POKEMON;
    }
    if(string_equals_ignore_case(mensaje, "APPEARED_POKEMON")) {
        return APPEARED_POKEMON;
    }
    if(string_equals_ignore_case(mensaje, "CATCH_POKEMON")) {
        return CATCH_POKEMON;
    }
    if(string_equals_ignore_case(mensaje, "CAUGHT_POKEMON")) {
        return CAUGHT_POKEMON;
    }
    return -1;
}
