//
// Created by utnso on 17/04/20.
//

#include "deserialization.h"

void test_deserializarRecibirTodos(char* ip, char* puerto) {

    int broker = ipc_escuchar_en(ip, puerto);

    int gameBoy = ipc_esperar_cliente(broker);

    while(ipc_hay_datos_para_recibir_de(gameBoy)) {

        t_paquete* paquete = ipc_recibir_de(gameBoy);

        switch(paquete->header->tipo_mensaje) {
        case STRING:
            printf("testing string\n");
            char* string = paquete->payload;
            printf("string: %s\n\n", string);
            break;
        case NEW_POKEMON:
            printf("testing new_pokemon\n");
            t_new_pokemon* new_pokemon = paquete_to_new_pokemon(paquete);
            printf("tipo_mensaje:%d\n", paquete->header->tipo_mensaje);
            printf("payload_size:%d\n", paquete->header->payload_size);
            printf("nombre_len:%d\n", new_pokemon->nombre_len);
            printf("nombre:%s\n", new_pokemon->nombre);
            printf("posicion: (%d,%d)\n", new_pokemon->posicion->x, new_pokemon->posicion->y);
            printf("cantidad:%d\n\n", new_pokemon->cantidad);
            mensaje_liberar_new_pokemon(new_pokemon);
            break;
        case LOCALIZED_POKEMON:
            printf("testing localized_pokemon\n");
            t_localized_pokemon* localized_pokemon = paquete_to_localized_pokemon(paquete);
            printf("tipo_mensaje:%d\n", paquete->header->tipo_mensaje);
            printf("payload_size:%d\n", paquete->header->payload_size);
            printf("nombre_len:%d\n", localized_pokemon->nombre_len);
            printf("nombre:%s\n", localized_pokemon->nombre);
            printf("cantidad_posiciones:%d\n", localized_pokemon->posiciones_len);
            printf("posiciones: ");
            for(int i = 0; i < localized_pokemon->posiciones_len; i++) {
                t_coord* posicion = (t_coord*) list_get(localized_pokemon->posiciones, i);
                if(i > 0) {
                    printf(", ");
                }
                printf("(%d,%d)", posicion->x, posicion->y);
            }
            printf("\n\n");
            mensaje_liberar_localized_pokemon(localized_pokemon);
            break;
        case GET_POKEMON:
            printf("testing get_pokemon\n");
            t_get_pokemon* get_pokemon = paquete_to_get_pokemon(paquete);
            printf("tipo_mensaje:%d\n", paquete->header->tipo_mensaje);
            printf("payload_size:%d\n", paquete->header->payload_size);
            printf("nombre_len:%d\n", get_pokemon->nombre_len);
            printf("nombre:%s\n\n", get_pokemon->nombre);
            mensaje_liberar_get_pokemon(get_pokemon);
            break;
        case APPEARED_POKEMON:
            printf("testing appeared_pokemon\n");
            t_appeared_pokemon* appeared_pokemon = paquete_to_appeared_pokemon(paquete);
            printf("tipo_mensaje:%d\n", paquete->header->tipo_mensaje);
            printf("payload_size:%d\n", paquete->header->payload_size);
            printf("nombre_len:%d\n", appeared_pokemon->nombre_len);
            printf("nombre:%s\n", appeared_pokemon->nombre);
            printf("posicion: (%d,%d)\n\n", appeared_pokemon->posicion->x, appeared_pokemon->posicion->y);
            mensaje_liberar_appeared_pokemon(appeared_pokemon);
            break;
        case CATCH_POKEMON:
            printf("testing catch_pokemon\n");
            t_catch_pokemon* catch_pokemon = paquete_to_catch_pokemon(paquete);
            printf("tipo_mensaje:%d\n", paquete->header->tipo_mensaje);
            printf("payload_size:%d\n", paquete->header->payload_size);
            printf("nombre_len:%d\n", catch_pokemon->nombre_len);
            printf("nombre:%s\n", catch_pokemon->nombre);
            printf("posicion: (%d,%d)\n\n", catch_pokemon->posicion->x, catch_pokemon->posicion->y);
            mensaje_liberar_catch_pokemon(catch_pokemon);
            break;
        case CAUGHT_POKEMON:
            printf("testing caught_pokemon\n");
            t_caught_pokemon* caught_pokemon = paquete_to_caught_pokemon(paquete);
            printf("tipo_mensaje:%d\n", paquete->header->tipo_mensaje);
            printf("payload_size:%d\n", paquete->header->payload_size);
            printf("is_caught: %s\n\n", caught_pokemon->is_caught ? "true" : "false");
            mensaje_liberar_caught_pokemon(caught_pokemon);
            break;
        default:
            printf("Esta llegando cualquier cosa\n");
        }

        paquete_liberar(paquete);
    }

    ipc_cerrar(gameBoy);
}
