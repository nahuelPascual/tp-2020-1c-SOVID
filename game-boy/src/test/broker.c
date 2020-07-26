/*
 * broker.c
 *
 *  Created on: 17 may. 2020
 *      Author: utnso
 */

#include "broker.h"

void _esperar_informe_id_y_morir(int broker) {
    printf("################################################\n");
    printf("Recibiendo id correspondiente al mensaje enviado\n");
    printf("################################################\n\n");

    while(ipc_hay_datos_para_recibir_de(broker)) {
        t_paquete* paquete_respuesta = ipc_recibir_de(broker);
        t_informe_id* informe_id = paquete_to_informe_id(paquete_respuesta);

        printf("tipo_paquete: %d\n", paquete_respuesta->header->tipo_paquete);
        printf("tipo_mensaje: %d\n", paquete_respuesta->header->tipo_mensaje);
        printf("id_mensaje: %d\n", paquete_respuesta->header->id_mensaje);
        printf("correlation_id_mensaje: %d\n", paquete_respuesta->header->correlation_id_mensaje);
        printf("payload_size: %d\n", paquete_respuesta->header->payload_size);
        printf("id_mensaje: %d\n\n", informe_id->id_mensaje);

        paquete_liberar(paquete_respuesta);
        informe_id_liberar(informe_id);

        ipc_cerrar(broker);
    }
}

void enviar_new_pokemon(int broker) {
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

    ipc_enviar_a(broker, paquete);

    _esperar_informe_id_y_morir(broker);

    mensaje_liberar_new_pokemon(pokemon);
    paquete_liberar(paquete);
}

void enviar_localized_pokemon(int broker) {
    printf("testing localized_pokemon\n");
    char* name = "Pikachu";

    t_list* coordenadas = list_create();
    t_coord* coordenada_1 = malloc(sizeof(t_coord));
    t_coord* coordenada_2 = malloc(sizeof(t_coord));
    coordenada_1->x = 1;
    coordenada_1->y = 1;
    coordenada_2->x = 6;
    coordenada_2->y = 6;
    list_add(coordenadas, coordenada_1);
    list_add(coordenadas, coordenada_2);

    t_localized_pokemon* pokemon = mensaje_crear_localized_pokemon(name, 2, coordenadas);
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

    ipc_enviar_a(broker, paquete);

    _esperar_informe_id_y_morir(broker);

    mensaje_liberar_localized_pokemon(pokemon);
    paquete_liberar(paquete);
}

void enviar_get_pokemon(int broker) {
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

    ipc_enviar_a(broker, paquete);

    _esperar_informe_id_y_morir(broker);

    mensaje_liberar_get_pokemon(pokemon);
    paquete_liberar(paquete);
}

void enviar_appeared_pokemon(int broker) {
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

    ipc_enviar_a(broker, paquete);

    _esperar_informe_id_y_morir(broker);

    mensaje_liberar_appeared_pokemon(pokemon);
    paquete_liberar(paquete);
}

void enviar_catch_pokemon(int broker) {
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

    ipc_enviar_a(broker, paquete);

    _esperar_informe_id_y_morir(broker);

    mensaje_liberar_catch_pokemon(pokemon);
    paquete_liberar(paquete);
}

void enviar_caught_pokemon(int broker) {
    printf("testing caught_pokemon\n");
    t_caught_pokemon* pokemon = mensaje_crear_caught_pokemon(1);

    t_paquete* paquete = paquete_from_caught_pokemon(pokemon);
    printf("tipo_paquete: %d\n", paquete->header->tipo_paquete);
    printf("tipo_mensaje: %d\n", paquete->header->tipo_mensaje);
    printf("id_mensaje: %d\n", paquete->header->id_mensaje);
    printf("correlation_id_mensaje: %d\n", paquete->header->correlation_id_mensaje);
    printf("payload_size: %d\n", paquete->header->payload_size);
    printf("is_caught: %s\n\n", pokemon->is_caught ? "true" : "false");

    ipc_enviar_a(broker, paquete);

    _esperar_informe_id_y_morir(broker);

    mensaje_liberar_caught_pokemon(pokemon);
    paquete_liberar(paquete);
}

void enviar_informe_id(int broker) {
    printf("testing informe_id\n");
    t_informe_id* informe_id = informe_id_crear(1);

    t_paquete* paquete = paquete_from_informe_id(informe_id);
    printf("tipo_paquete: %d\n", paquete->header->tipo_paquete);
    printf("tipo_mensaje: %d\n", paquete->header->tipo_mensaje);
    printf("id_mensaje: %d\n", paquete->header->id_mensaje);
    printf("correlation_id_mensaje: %d\n", paquete->header->correlation_id_mensaje);
    printf("payload_size: %d\n", paquete->header->payload_size);
    printf("id_mensaje: %d\n\n", informe_id->id_mensaje);

    ipc_enviar_a(broker, paquete);

    informe_id_liberar(informe_id);
    paquete_liberar(paquete);
}

void enviar_ack(int broker, uint32_t id_suscriptor, uint32_t id_mensaje) {
    printf("testing ack\n");
    t_ack* ack = ack_crear(id_suscriptor, id_mensaje);

    t_paquete* paquete = paquete_from_ack(ack);
    printf("tipo_paquete: %d\n", paquete->header->tipo_paquete);
    printf("tipo_mensaje: %d\n", paquete->header->tipo_mensaje);
    printf("id_mensaje: %d\n", paquete->header->id_mensaje);
    printf("correlation_id_mensaje: %d\n", paquete->header->correlation_id_mensaje);
    printf("payload_size: %d\n", paquete->header->payload_size);
    printf("id_suscriptor: %d\n", ack->id_suscriptor);
    printf("id_mensaje: %d\n\n", ack->id_mensaje);

    ipc_enviar_a(broker, paquete);

    paquete_liberar(paquete);
    ack_liberar(ack);
}

void enviar_suscripcion(int broker, t_tipo_mensaje tipo_mensaje, uint32_t id_suscriptor) {
    printf("testing suscripcion\n");
    t_suscripcion* suscripcion = suscripcion_crear(tipo_mensaje, id_suscriptor, 0);
    t_paquete* paquete = paquete_from_suscripcion(suscripcion);
    printf("tipo_paquete: %d\n", paquete->header->tipo_paquete);
    printf("tipo_mensaje: %d\n", paquete->header->tipo_mensaje);
    printf("id_mensaje: %d\n", paquete->header->id_mensaje);
    printf("correlation_id_mensaje: %d\n", paquete->header->correlation_id_mensaje);
    printf("payload_size: %d\n", paquete->header->payload_size);
    printf("tipo_mensaje: %d\n", suscripcion->tipo_mensaje);
    printf("id_suscriptor: %d\n", suscripcion->id_suscriptor);
    printf("tiempo: %d\n\n", suscripcion->tiempo);

    ipc_enviar_a(broker, paquete);

    printf("########################################\n");
    printf("Recibiendo mensajes de la cola suscripta\n");
    printf("########################################\n\n");

    while(ipc_hay_datos_para_recibir_de(broker)) {

        t_paquete* paquete_respuesta = ipc_recibir_de(broker);

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

        //Prueba respondiendo por el mismo socket -> requiere while en _gestionar_a
        enviar_ack(broker, id_suscriptor, paquete_respuesta->header->id_mensaje);
    }

    suscripcion_liberar(suscripcion);
    paquete_liberar(paquete);
}

void test_broker(char**argv, char* ip, char* puerto) {
    int broker = ipc_conectarse_a(ip, puerto);

    if(string_equals_ignore_case(argv[1], "new")) {
        enviar_new_pokemon(broker);
    }
    else if(string_equals_ignore_case(argv[1], "localized")) {
        enviar_localized_pokemon(broker);
    }
    else if(string_equals_ignore_case(argv[1], "get")) {
        enviar_get_pokemon(broker);
    }
    else if(string_equals_ignore_case(argv[1], "appeared")) {
        enviar_appeared_pokemon(broker);
    }
    else if(string_equals_ignore_case(argv[1], "catch")) {
        enviar_catch_pokemon(broker);
    }
    else if(string_equals_ignore_case(argv[1], "caught")) {
        enviar_caught_pokemon(broker);
    }
    else if(string_equals_ignore_case(argv[1], "informe_id")) {
        enviar_informe_id(broker);
        ipc_cerrar(broker);
    }
    //Prueba respondiendo por otro socket -> requiere if en _gestionar_a
    else if(string_equals_ignore_case(argv[1], "ack")) {
        enviar_ack(broker, atoi(argv[2]), atoi(argv[3]));
        ipc_cerrar(broker);
    }
    else if(string_equals_ignore_case(argv[1], "suscripcion")) {
        enviar_suscripcion(broker, atoi(argv[2]), atoi(argv[3]));
    }
}
