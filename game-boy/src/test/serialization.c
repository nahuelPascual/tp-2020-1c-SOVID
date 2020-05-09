//
// Created by utnso on 17/04/20.
//

#include "serialization.h"

void test_new_pokemon(int conn) {
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

    ipc_enviar_a(conn, paquete);

    mensaje_liberar_new_pokemon(pokemon);
    paquete_liberar(paquete);
}

void test_localized_pokemon(int conn) {
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

    ipc_enviar_a(conn, paquete);

    mensaje_liberar_localized_pokemon(pokemon);
    paquete_liberar(paquete);
}

void test_get_pokemon(int conn) {
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

    ipc_enviar_a(conn, paquete);

    mensaje_liberar_get_pokemon(pokemon);
    paquete_liberar(paquete);
}

void test_appeared_pokemon(int conn) {
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

    ipc_enviar_a(conn, paquete);

    mensaje_liberar_appeared_pokemon(pokemon);
    paquete_liberar(paquete);
}

void test_catch_pokemon(int conn) {
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

    ipc_enviar_a(conn, paquete);

    mensaje_liberar_catch_pokemon(pokemon);
    paquete_liberar(paquete);
}

void test_caught_pokemon(int conn) {
    printf("testing caught_pokemon\n");
    t_caught_pokemon* pokemon = mensaje_crear_caught_pokemon(1);

    t_paquete* paquete = paquete_from_caught_pokemon(pokemon);
    printf("tipo_paquete: %d\n", paquete->header->tipo_paquete);
    printf("tipo_mensaje: %d\n", paquete->header->tipo_mensaje);
    printf("id_mensaje: %d\n", paquete->header->id_mensaje);
    printf("correlation_id_mensaje: %d\n", paquete->header->correlation_id_mensaje);
    printf("payload_size: %d\n", paquete->header->payload_size);
    printf("is_caught: %s\n\n", pokemon->is_caught ? "true" : "false");

    ipc_enviar_a(conn, paquete);

    mensaje_liberar_caught_pokemon(pokemon);
    paquete_liberar(paquete);
}

void test_ack(int conn) {
    printf("testing ack\n");
    t_ack* ack = ack_crear(1);

    t_paquete* paquete = paquete_from_ack(ack);
    printf("tipo_paquete: %d\n", paquete->header->tipo_paquete);
    printf("tipo_mensaje: %d\n", paquete->header->tipo_mensaje);
    printf("id_mensaje: %d\n", paquete->header->id_mensaje);
    printf("correlation_id_mensaje: %d\n", paquete->header->correlation_id_mensaje);
    printf("payload_size: %d\n", paquete->header->payload_size);
    printf("id_mensaje: %d\n\n", ack->id_mensaje);

    ipc_enviar_a(conn, paquete);

    ack_liberar(ack);
    paquete_liberar(paquete);

}

void test_suscripcion(int conn) {
    printf("testing suscripcion\n");
    t_suscripcion* suscripcion = suscripcion_crear(NEW_POKEMON, 10);

    t_paquete* paquete = paquete_from_suscripcion(suscripcion);
    printf("tipo_paquete: %d\n", paquete->header->tipo_paquete);
    printf("tipo_mensaje: %d\n", paquete->header->tipo_mensaje);
    printf("id_mensaje: %d\n", paquete->header->id_mensaje);
    printf("correlation_id_mensaje: %d\n", paquete->header->correlation_id_mensaje);
    printf("payload_size: %d\n", paquete->header->payload_size);
    printf("tipo_mensaje: %d\n", suscripcion->tipo_mensaje);
    printf("tiempo: %d\n\n", suscripcion->tiempo);

    ipc_enviar_a(conn, paquete);

    suscripcion_liberar(suscripcion);
    paquete_liberar(paquete);
}

void test_informe_id(int conn) {
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

void test_serializarEnviarTodos(char* ip, char* puerto) {
    int conn = ipc_conectarse_a(ip, puerto);

    test_new_pokemon(conn);
    test_localized_pokemon(conn);
    test_get_pokemon(conn);
    test_appeared_pokemon(conn);
    test_catch_pokemon(conn);
    test_caught_pokemon(conn);
    test_ack(conn);
    test_suscripcion(conn);
    test_informe_id(conn);

    ipc_cerrar(conn);
}
