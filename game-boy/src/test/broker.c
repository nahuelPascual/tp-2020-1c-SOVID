/*
 * broker.c
 *
 *  Created on: 17 may. 2020
 *      Author: utnso
 */

#include "broker.h"

void enviar_new_pokemon(int cliente) {
    printf("testing new_pokemon\n");
    char* name = "Pikachu";

    t_new_pokemon* pokemon = mensaje_crear_new_pokemon(name, 15, 2, 1);

    t_paquete* paquete = paquete_from_new_pokemon(pokemon);
    printf("tipo_paquete: %d\n", paquete->header->tipo_paquete);
    printf("tipo_mensaje: %d\n", paquete->header->tipo_mensaje);
    printf("id_mensaje: %d\n", paquete->header->id_mensaje);
    printf("correlation_id_mensaje: %d\n", paquete->header->correlation_id_mensaje);
    printf("payload_size: %d\n", paquete->header->payload_size);
    printf("nombre_len: %d\n", pokemon->nombre_len);
    printf("nombre: %s\n", pokemon->nombre);
    printf("posicion: (%d,%d)\n", pokemon->posicion->x, pokemon->posicion->y);
    printf("cantidad: %d\n\n", pokemon->cantidad);

    ipc_enviar_a(cliente, paquete);

    mensaje_liberar_new_pokemon(pokemon);
    paquete_liberar(paquete);
}

void enviar_localized_pokemon(int cliente) {
    printf("testing localized_pokemon\n");
    char* name = "Pikachu";

    t_localized_pokemon* pokemon = mensaje_crear_localized_pokemon(name, 2, 1, 1, 6, 6);
    t_paquete* paquete = paquete_from_localized_pokemon(pokemon);
    printf("tipo_paquete: %d\n", paquete->header->tipo_paquete);
    printf("tipo_mensaje: %d\n", paquete->header->tipo_mensaje);
    printf("id_mensaje: %d\n", paquete->header->id_mensaje);
    printf("correlation_id_mensaje: %d\n", paquete->header->correlation_id_mensaje);
    printf("payload_size: %d\n", paquete->header->payload_size);
    printf("nombre_len: %d\n", pokemon->nombre_len);
    printf("nombre: %s\n", pokemon->nombre);
    printf("cantidad_posiciones: %d\n", pokemon->posiciones_len);
    printf("posiciones: ");
    for(int j = 0; j < pokemon->posiciones_len; j++) {
        t_coord* pos = (t_coord*) list_get(pokemon->posiciones, j);
        if(j > 0) {
            printf(", ");
        }
        printf("(%d,%d)", pos->x, pos->y);
    }
    printf("\n\n");

    ipc_enviar_a(cliente, paquete);

    mensaje_liberar_localized_pokemon(pokemon);
    paquete_liberar(paquete);
}

void enviar_get_pokemon(int cliente) {
    printf("testing get_pokemon\n");
    char* name = "Pikachu";

    t_get_pokemon* pokemon = mensaje_crear_get_pokemon(name);

    t_paquete* paquete = paquete_from_get_pokemon(pokemon);
    printf("tipo_paquete: %d\n", paquete->header->tipo_paquete);
    printf("tipo_mensaje: %d\n", paquete->header->tipo_mensaje);
    printf("id_mensaje: %d\n", paquete->header->id_mensaje);
    printf("correlation_id_mensaje: %d\n", paquete->header->correlation_id_mensaje);
    printf("payload_size: %d\n", paquete->header->payload_size);
    printf("nombre_len: %d\n", pokemon->nombre_len);
    printf("nombre: %s\n\n", pokemon->nombre);

    ipc_enviar_a(cliente, paquete);

    mensaje_liberar_get_pokemon(pokemon);
    paquete_liberar(paquete);
}

void enviar_appeared_pokemon(int cliente) {
    printf("testing appeared_pokemon\n");
    char* name = "Pikachu";

    t_appeared_pokemon* pokemon = mensaje_crear_appeared_pokemon(name, 3, 2);

    t_paquete* paquete = paquete_from_appeared_pokemon(pokemon);
    printf("tipo_paquete: %d\n", paquete->header->tipo_paquete);
    printf("tipo_mensaje: %d\n", paquete->header->tipo_mensaje);
    printf("id_mensaje: %d\n", paquete->header->id_mensaje);
    printf("correlation_id_mensaje: %d\n", paquete->header->correlation_id_mensaje);
    printf("payload_size: %d\n", paquete->header->payload_size);
    printf("nombre_len: %d\n", pokemon->nombre_len);
    printf("nombre: %s\n", pokemon->nombre);
    printf("posicion: (%d,%d)\n\n", pokemon->posicion->x, pokemon->posicion->y);

    ipc_enviar_a(cliente, paquete);

    mensaje_liberar_appeared_pokemon(pokemon);
    paquete_liberar(paquete);
}

void enviar_catch_pokemon(int cliente) {
    printf("testing catch_pokemon\n");
    char* name = "Pikachu";

    t_catch_pokemon* pokemon = mensaje_crear_catch_pokemon(name, 3, 2);

    t_paquete* paquete = paquete_from_catch_pokemon(pokemon);
    printf("tipo_paquete: %d\n", paquete->header->tipo_paquete);
    printf("tipo_mensaje: %d\n", paquete->header->tipo_mensaje);
    printf("id_mensaje: %d\n", paquete->header->id_mensaje);
    printf("correlation_id_mensaje: %d\n", paquete->header->correlation_id_mensaje);
    printf("payload_size: %d\n", paquete->header->payload_size);
    printf("nombre_len: %d\n", pokemon->nombre_len);
    printf("nombre: %s\n", pokemon->nombre);
    printf("posicion: (%d,%d)\n\n", pokemon->posicion->x, pokemon->posicion->y);

    ipc_enviar_a(cliente, paquete);

    mensaje_liberar_catch_pokemon(pokemon);
    paquete_liberar(paquete);
}

void enviar_caught_pokemon(int cliente) {
    printf("testing caught_pokemon\n");
    t_caught_pokemon* pokemon = mensaje_crear_caught_pokemon(1);

    t_paquete* paquete = paquete_from_caught_pokemon(pokemon);
    printf("tipo_paquete: %d\n", paquete->header->tipo_paquete);
    printf("tipo_mensaje: %d\n", paquete->header->tipo_mensaje);
    printf("id_mensaje: %d\n", paquete->header->id_mensaje);
    printf("correlation_id_mensaje: %d\n", paquete->header->correlation_id_mensaje);
    printf("payload_size: %d\n", paquete->header->payload_size);
    printf("is_caught: %s\n\n", pokemon->is_caught ? "true" : "false");

    ipc_enviar_a(cliente, paquete);

    mensaje_liberar_caught_pokemon(pokemon);
    paquete_liberar(paquete);
}

void enviar_informe_id(int conn) {
    printf("testing informe_id\n");
    t_informe_id* informe_id = informe_id_crear(1);

    t_paquete* paquete = paquete_from_informe_id(informe_id);
    printf("tipo_paquete: %d\n", paquete->header->tipo_paquete);
    printf("tipo_mensaje: %d\n", paquete->header->tipo_mensaje);
    printf("id_mensaje: %d\n", paquete->header->id_mensaje);
    printf("correlation_id_mensaje: %d\n", paquete->header->correlation_id_mensaje);
    printf("payload_size: %d\n", paquete->header->payload_size);
    printf("id_mensaje: %d\n\n", informe_id->id_mensaje);

    ipc_enviar_a(conn, paquete);

    informe_id_liberar(informe_id);
    paquete_liberar(paquete);
}

void _responder_ack(int cliente, uint32_t id_mensaje) {
    printf("testing ack\n");
    t_ack* ack = ack_crear(id_mensaje);

    t_paquete* paquete = paquete_from_ack(ack);
    printf("tipo_paquete: %d\n", paquete->header->tipo_paquete);
    printf("tipo_mensaje: %d\n", paquete->header->tipo_mensaje);
    printf("id_mensaje: %d\n", paquete->header->id_mensaje);
    printf("correlation_id_mensaje: %d\n", paquete->header->correlation_id_mensaje);
    printf("payload_size: %d\n", paquete->header->payload_size);
    printf("id_mensaje: %d\n\n", ack->id_mensaje);

    ipc_enviar_a(cliente, paquete);

    ack_liberar(ack);
    paquete_liberar(paquete);
}

void enviar_suscripcion(int cliente, t_tipo_mensaje tipo_mensaje) {
    printf("testing suscripcion\n");
    t_suscripcion* suscripcion = suscripcion_crear(tipo_mensaje, 0);
    t_paquete* paquete = paquete_from_suscripcion(suscripcion);
    printf("tipo_paquete: %d\n", paquete->header->tipo_paquete);
    printf("tipo_mensaje: %d\n", paquete->header->tipo_mensaje);
    printf("id_mensaje: %d\n", paquete->header->id_mensaje);
    printf("correlation_id_mensaje: %d\n", paquete->header->correlation_id_mensaje);
    printf("payload_size: %d\n", paquete->header->payload_size);
    printf("tipo_mensaje: %d\n", suscripcion->tipo_mensaje);
    printf("tiempo: %d\n\n", suscripcion->tiempo);

    ipc_enviar_a(cliente, paquete);

    printf("########################################\n");
    printf("Recibiendo mensajes de la cola suscripta\n");
    printf("########################################\n\n");

    while(ipc_hay_datos_para_recibir_de(cliente)) {

        t_paquete* paquete_respuesta = ipc_recibir_de(cliente);

        switch(paquete_respuesta->header->tipo_mensaje) {
        case NEW_POKEMON:
            printf("Recibo un NEW_POKEMON con id_mensaje: %d\n", paquete_respuesta->header->id_mensaje);
            break;
        case LOCALIZED_POKEMON:
            printf("Recibo un LOCALIZED_POKEMON con id_mensaje: %d\n", paquete_respuesta->header->id_mensaje);
            break;
        case GET_POKEMON:
            printf("Recibo un GET_POKEMON con id_mensaje: %d\n", paquete_respuesta->header->id_mensaje);
            break;
        case APPEARED_POKEMON:
            printf("Recibo un APPEARED_POKEMON con id_mensaje: %d\n", paquete_respuesta->header->id_mensaje);
            break;
        case CATCH_POKEMON:
            printf("Recibo un CATCH_POKEMON con id_mensaje: %d\n", paquete_respuesta->header->id_mensaje);
            break;
        case CAUGHT_POKEMON:
            printf("Recibo un CAUGHT_POKEMON con id_mensaje: %d\n", paquete_respuesta->header->id_mensaje);
            break;
        default:
            printf("Esta llegando cualquier cosa\n");
            break;
        }

        _responder_ack(cliente, paquete_respuesta->header->id_mensaje);
    }

    suscripcion_liberar(suscripcion);
    paquete_liberar(paquete);
}

void test_broker(char**argv, char* ip, char* puerto) {
    int cliente = ipc_conectarse_a(ip, puerto);

    if(string_equals_ignore_case(argv[1], "new")) {
        enviar_new_pokemon(cliente);
        ipc_cerrar(cliente);
    }
    else if(string_equals_ignore_case(argv[1], "localized")) {
        enviar_localized_pokemon(cliente);
        ipc_cerrar(cliente);
    }
    else if(string_equals_ignore_case(argv[1], "get")) {
        enviar_get_pokemon(cliente);
        ipc_cerrar(cliente);
    }
    else if(string_equals_ignore_case(argv[1], "appeared")) {
        enviar_appeared_pokemon(cliente);
        ipc_cerrar(cliente);
    }
    else if(string_equals_ignore_case(argv[1], "catch")) {
        enviar_catch_pokemon(cliente);
        ipc_cerrar(cliente);
    }
    else if(string_equals_ignore_case(argv[1], "caught")) {
        enviar_caught_pokemon(cliente);
        ipc_cerrar(cliente);
    }
    else if(string_equals_ignore_case(argv[1], "informe_id")) {
        enviar_informe_id(cliente);
        ipc_cerrar(cliente);
    }
    else if(string_equals_ignore_case(argv[1], "suscripcion")) {
        enviar_suscripcion(cliente, atoi(argv[2]));
    }
}
