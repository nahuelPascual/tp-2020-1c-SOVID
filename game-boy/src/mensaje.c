//
// Created by utnso on 19/04/20.
//

#include "mensaje.h"

t_paquete* resolver_mensaje(int codOperacion, char** args) {
    void* msg;
    t_paquete* p;

    switch(codOperacion){
        case NEW_POKEMON:
            msg = mensaje_crear_new_pokemon(args[3], atoi(args[4]), atoi(args[5]), atoi(args[6]));
            p = paquete_from_new_pokemon((t_new_pokemon*)msg);
            if (args[7] != NULL){ // tiene id_mensaje cuando se envia al game-card
                p->header->id_mensaje = atoi(args[7]);
            }
            mensaje_liberar_new_pokemon(msg);
            return p;
        case GET_POKEMON:
            msg = mensaje_crear_get_pokemon(args[3]);
            p = paquete_from_get_pokemon((t_get_pokemon*)msg);
            if (args[4] != NULL) { // tiene id_mensaje cuando se envia al game-card
                p->header->id_mensaje = atoi(args[4]);
            }
            mensaje_liberar_get_pokemon(msg);
            return p;
        case APPEARED_POKEMON:
            msg = mensaje_crear_appeared_pokemon(args[3], atoi(args[4]), atoi(args[5]));
            p = paquete_from_appeared_pokemon((t_appeared_pokemon*)msg);
            if (args[6] != NULL){ // tiene correlation_id_mensaje cuando se envia al broker
                p->header->correlation_id_mensaje = atoi(args[6]);
            }
            mensaje_liberar_appeared_pokemon(msg);
            return p;
        case CATCH_POKEMON:
            msg = mensaje_crear_catch_pokemon(args[3], atoi(args[4]), atoi(args[5]));
            p = paquete_from_catch_pokemon((t_catch_pokemon*)msg);
            if (args[6] != NULL){ // tiene id_mensaje cuando se envia al game-card
                p->header->id_mensaje = atoi(args[6]);
            }
            mensaje_liberar_catch_pokemon(msg);
            return p;
        case CAUGHT_POKEMON:
            msg = mensaje_crear_caught_pokemon(atoi(args[4])); // por la consola nos mandan OK/FAIL o 1/0?
            p = paquete_from_caught_pokemon((t_caught_pokemon*)msg);
            p->header->correlation_id_mensaje = atoi(args[3]);
            mensaje_liberar_caught_pokemon(msg);
            return p;
    }

    return NULL;
}

int normalizar_tipo_mensaje(char* mensaje) {
    if (string_equals_ignore_case(mensaje, "NEW_POKEMON")){
        return NEW_POKEMON;
    }
    if (string_equals_ignore_case(mensaje, "LOCALIZED_POKEMON")){
        return LOCALIZED_POKEMON;
    }
    if (string_equals_ignore_case(mensaje, "GET_POKEMON")){
        return GET_POKEMON;
    }
    if (string_equals_ignore_case(mensaje, "APPEARED_POKEMON")){
        return APPEARED_POKEMON;
    }
    if (string_equals_ignore_case(mensaje, "CATCH_POKEMON")){
        return CATCH_POKEMON;
    }
    if (string_equals_ignore_case(mensaje, "CAUGHT_POKEMON")){
        return CAUGHT_POKEMON;
    }
    return -1;
}
