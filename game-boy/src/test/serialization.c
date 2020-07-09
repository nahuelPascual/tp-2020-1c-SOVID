//
// Created by utnso on 17/04/20.
//

#include "serialization.h"

t_log* logger;

void iniciar_tests(){

    logger = log_create("log_gameboy_debug.txt", "GAMEBOY", true, LOG_LEVEL_DEBUG);
    test_serializarEnviarTodos("BROKER", "127.0.0.1", "8081");
    log_destroy(logger);
}

void test_new_pokemon(int conn) {
    printf("Testing new_pokemon\n");
    char* name = "Pikachu";

    t_new_pokemon* pokemon = mensaje_crear_new_pokemon(name, 15, 2, 1);

    t_paquete* paquete = paquete_from_new_pokemon(pokemon);

    ipc_enviar_a(conn, paquete);
    logger_enviado(logger, paquete);

    mensaje_liberar_new_pokemon(pokemon);
    paquete_liberar(paquete);
}

void test_localized_pokemon(int conn) {
    printf("Testing localized_pokemon\n");
    char* name = "Pikachu";

    t_localized_pokemon* pokemon = mensaje_crear_localized_pokemon(name, 2, 1, 1, 6, 6);
    t_paquete* paquete = paquete_from_localized_pokemon(pokemon);

    ipc_enviar_a(conn, paquete);
    logger_enviado(logger, paquete);

    mensaje_liberar_localized_pokemon(pokemon);
    paquete_liberar(paquete);
}

void test_get_pokemon(int conn) {
    printf("Testing get_pokemon\n");
    char* name = "Pikachu";

    t_get_pokemon* pokemon = mensaje_crear_get_pokemon(name);

    t_paquete* paquete = paquete_from_get_pokemon(pokemon);

    ipc_enviar_a(conn, paquete);
    logger_enviado(logger, paquete);

    mensaje_liberar_get_pokemon(pokemon);
    paquete_liberar(paquete);
}

void test_appeared_pokemon(int conn) {
    printf("Testing appeared_pokemon\n");
    char* name = "Pikachu";

    t_appeared_pokemon* pokemon = mensaje_crear_appeared_pokemon(name, 3, 2);

    t_paquete* paquete = paquete_from_appeared_pokemon(pokemon);

    ipc_enviar_a(conn, paquete);
    logger_enviado(logger, paquete);

    mensaje_liberar_appeared_pokemon(pokemon);
    paquete_liberar(paquete);
}

void test_catch_pokemon(int conn) {
    printf("Testing catch_pokemon\n");
    char* name = "Pikachu";

    t_catch_pokemon* pokemon = mensaje_crear_catch_pokemon(name, 3, 2);

    t_paquete* paquete = paquete_from_catch_pokemon(pokemon);

    ipc_enviar_a(conn, paquete);
    logger_enviado(logger, paquete);

    mensaje_liberar_catch_pokemon(pokemon);
    paquete_liberar(paquete);
}

void test_caught_pokemon(int conn) {
    printf("Testing caught_pokemon\n");
    t_caught_pokemon* pokemon = mensaje_crear_caught_pokemon(1);

    t_paquete* paquete = paquete_from_caught_pokemon(pokemon);

    ipc_enviar_a(conn, paquete);
    logger_enviado(logger, paquete);

    mensaje_liberar_caught_pokemon(pokemon);
    paquete_liberar(paquete);
}

void test_ack(int conn) {
    printf("testing ack\n");
    t_ack* ack = ack_crear(1, 1);

    t_paquete* paquete = paquete_from_ack(ack);

    ipc_enviar_a(conn, paquete);
    logger_enviado(logger, paquete);

    ack_liberar(ack);
    paquete_liberar(paquete);

}

void test_suscripcion(int conn) {
    printf("testing suscripcion\n");
    t_suscripcion* suscripcion = suscripcion_crear(NEW_POKEMON, 1, 10);

    t_paquete* paquete = paquete_from_suscripcion(suscripcion);

    ipc_enviar_a(conn, paquete);
    logger_enviado(logger, paquete);

    suscripcion_liberar(suscripcion);
    paquete_liberar(paquete);
}

void test_informe_id(int conn) {
    printf("Testing informe_id\n");
    t_informe_id* informe_id = informe_id_crear(1);

    t_paquete* paquete = paquete_from_informe_id(informe_id);

    ipc_enviar_a(conn, paquete);
    logger_enviado(logger, paquete);

    informe_id_liberar(informe_id);
    paquete_liberar(paquete);
}

void test_serializarEnviarTodos(char* proceso, char* ip, char* puerto) {
    int conn = ipc_conectarse_a(ip, puerto);
    logger_conexion(logger, proceso, ip, puerto);

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
