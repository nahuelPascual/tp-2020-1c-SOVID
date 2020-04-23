//
// Created by utnso on 17/04/20.
//

#include "serialization.h"

void test_string(int conn){
    printf("testing string");
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->header = malloc(sizeof(t_header));
    uint32_t cod_op = STRING;

    char* str = "Hola capo, vengo del futuro y queria contarte que aprobaste Operativos en el 1C2020";

    int size = strlen(str)+1;
    void* stream = malloc (size);

    memcpy(stream, str, size);

    paquete->header->tipo_mensaje = cod_op;
    paquete->header->payload_size = strlen(str)+1;
    paquete->payload = stream;

    ipc_enviar_a(conn, paquete);
    paquete_liberar(paquete);
}

void test_new_pokemon(int conn){
    printf("testing new_pokemon\n");
    char* name = "Pikachu";

    t_new_pokemon* pokemon = crear_new_pokemon(name, 15, 2, 1);

    t_paquete* paquete = mensaje_paquete_from_new_pokemon(pokemon);
    printf("tipo_mensaje:%d - payload_size:%d\n\n", paquete->header->tipo_mensaje, paquete->header->payload_size);

    free(pokemon->posicion);
    free(pokemon);

    ipc_enviar_a(conn, paquete);
    paquete_liberar(paquete);
}

void test_localized_pokemon(int conn){
    printf("testing localized_pokemon\n");
	char* name = "Pikachu";

    t_localized_pokemon* pokemon = crear_localized_pokemon(name, 2, 1, 1, 6, 6);
    for (int i = 0; i < pokemon->posiciones_len ; ++i) {
        t_coord* c = (t_coord*) list_get(pokemon->posiciones, i);
        printf("x:%d - y:%d\n", c->x, c->y);
    }
    t_paquete* paquete = mensaje_paquete_from_localized_pokemon(pokemon);
    printf("tipo_mensaje:%d - payload_size:%d\n\n", paquete->header->tipo_mensaje, paquete->header->payload_size);

    list_clean_and_destroy_elements(pokemon->posiciones, free);
    free(pokemon->posiciones);
    free(pokemon);

    ipc_enviar_a(conn, paquete);
    paquete_liberar(paquete);
}

void test_get_pokemon(int conn){
    printf("testing get_pokemon\n");
    char* name = "Pikachu";

    t_get_pokemon * pokemon = crear_get_pokemon(name);

    t_paquete* paquete = mensaje_paquete_from_get_pokemon(pokemon);
    printf("tipo_mensaje:%d - payload_size:%d\n\n", paquete->header->tipo_mensaje, paquete->header->payload_size);

    free(pokemon);

    ipc_enviar_a(conn, paquete);
    paquete_liberar(paquete);
}

void test_appeared_pokemon(int conn){
    printf("testing appeared_pokemon\n");
    char* name = "Pikachu";

    t_appeared_pokemon * pokemon = crear_appeared_pokemon(name, 3, 2);

    t_paquete* paquete = mensaje_paquete_from_appeared_pokemon(pokemon);
    printf("tipo_mensaje:%d - payload_size:%d\n\n", paquete->header->tipo_mensaje, paquete->header->payload_size);

    free(pokemon->posicion);
    free(pokemon);

    ipc_enviar_a(conn, paquete);
    paquete_liberar(paquete);
}

void test_catch_pokemon(int conn){
    printf("testing catch_pokemon\n");
    char* name = "Pikachu";

    t_catch_pokemon * pokemon = crear_catch_pokemon(name, 3, 2);

    t_paquete* paquete = mensaje_paquete_from_catch_pokemon(pokemon);
    printf("tipo_mensaje:%d - payload_size:%d\n\n", paquete->header->tipo_mensaje, paquete->header->payload_size);

    free(pokemon->posicion);
    free(pokemon);

    ipc_enviar_a(conn, paquete);
    paquete_liberar(paquete);
}

void test_caught_pokemon(int conn){
    printf("testing caught_pokemon\n");
    t_caught_pokemon * pokemon = crear_caught_pokemon(1);

    t_paquete* paquete = mensaje_paquete_from_caught_pokemon(pokemon);
    printf("tipo_mensaje:%d - payload_size:%d\n\n", paquete->header->tipo_mensaje, paquete->header->payload_size);

    free(pokemon);

    ipc_enviar_a(conn, paquete);
    paquete_liberar(paquete);
}

void test_serializarEnviarTodos(char* ip, char* puerto){
    int conn = ipc_conectarse_a(ip, puerto);

    test_string(conn);
    test_new_pokemon(conn);
    test_localized_pokemon(conn);
    test_get_pokemon(conn);
    test_appeared_pokemon(conn);
    test_catch_pokemon(conn);
    test_caught_pokemon(conn);

    ipc_cerrar(conn);
}
