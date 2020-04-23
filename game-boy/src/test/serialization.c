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
    printf("string: %s\n\n", str);
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

    t_new_pokemon* pokemon = mensaje_crear_new_pokemon(name, 15, 2, 1);

    t_paquete* paquete = paquete_from_new_pokemon(pokemon);
    printf("tipo_mensaje:%d\n", paquete->header->tipo_mensaje);
    printf("payload_size:%d\n", paquete->header->payload_size);
    printf("nombre_len:%d\n", pokemon->nombre_len);
    printf("nombre:%s\n", pokemon->nombre);
    printf("posicion: (%d,%d)\n", pokemon->posicion->x, pokemon->posicion->y);
    printf("cantidad:%d\n\n", pokemon->cantidad);

    free(pokemon->posicion);
    free(pokemon);

    ipc_enviar_a(conn, paquete);
    paquete_liberar(paquete);
}

void test_localized_pokemon(int conn){
    printf("testing localized_pokemon\n");
	char* name = "Pikachu";

    t_localized_pokemon* pokemon = mensaje_crear_localized_pokemon(name, 2, 1, 1, 6, 6);
    for (int i = 0; i < pokemon->posiciones_len ; ++i) {
        t_coord* c = (t_coord*) list_get(pokemon->posiciones, i);
    }
    t_paquete* paquete = paquete_from_localized_pokemon(pokemon);
    printf("tipo_mensaje:%d\n", paquete->header->tipo_mensaje);
    printf("payload_size:%d\n", paquete->header->payload_size);
    printf("nombre_len:%d\n", pokemon->nombre_len);
    printf("nombre:%s\n", pokemon->nombre);
    printf("cantidad_posiciones:%d\n", pokemon->posiciones_len);
    printf("posiciones: ");
    for (int j=0; j<pokemon->posiciones_len ; j++) {
        t_coord* pos = (t_coord*) list_get(pokemon->posiciones, j);
        if (j>0){
            printf(", ");
        }
        printf("(%d,%d)", pos->x, pos->y);
    }
    printf("\n\n");

    list_clean_and_destroy_elements(pokemon->posiciones, free);
    free(pokemon->posiciones);
    free(pokemon);

    ipc_enviar_a(conn, paquete);
    paquete_liberar(paquete);
}

void test_get_pokemon(int conn){
    printf("testing get_pokemon\n");
    char* name = "Pikachu";

    t_get_pokemon * pokemon = mensaje_crear_get_pokemon(name);

    t_paquete* paquete = paquete_from_get_pokemon(pokemon);
    printf("tipo_mensaje:%d\n", paquete->header->tipo_mensaje);
    printf("payload_size:%d\n", paquete->header->payload_size);
    printf("nombre_len:%d\n", pokemon->nombre_len);
    printf("nombre:%s\n\n", pokemon->nombre);

    free(pokemon);

    ipc_enviar_a(conn, paquete);
    paquete_liberar(paquete);
}

void test_appeared_pokemon(int conn){
    printf("testing appeared_pokemon\n");
    char* name = "Pikachu";

    t_appeared_pokemon * pokemon = mensaje_crear_appeared_pokemon(name, 3, 2);

    t_paquete* paquete = paquete_from_appeared_pokemon(pokemon);
    printf("tipo_mensaje:%d\n", paquete->header->tipo_mensaje);
    printf("payload_size:%d\n", paquete->header->payload_size);
    printf("nombre_len:%d\n", pokemon->nombre_len);
    printf("nombre:%s\n", pokemon->nombre);
    printf("posicion: (%d,%d)\n\n", pokemon->posicion->x, pokemon->posicion->y);

    free(pokemon->posicion);
    free(pokemon);

    ipc_enviar_a(conn, paquete);
    paquete_liberar(paquete);
}

void test_catch_pokemon(int conn){
    printf("testing catch_pokemon\n");
    char* name = "Pikachu";

    t_catch_pokemon * pokemon = mensaje_crear_catch_pokemon(name, 3, 2);

    t_paquete* paquete = paquete_from_catch_pokemon(pokemon);
    printf("tipo_mensaje:%d\n", paquete->header->tipo_mensaje);
    printf("payload_size:%d\n", paquete->header->payload_size);
    printf("nombre_len:%d\n", pokemon->nombre_len);
    printf("nombre:%s\n", pokemon->nombre);
    printf("posicion: (%d,%d)\n\n", pokemon->posicion->x, pokemon->posicion->y);

    free(pokemon->posicion);
    free(pokemon);

    ipc_enviar_a(conn, paquete);
    paquete_liberar(paquete);
}

void test_caught_pokemon(int conn){
    printf("testing caught_pokemon\n");
    t_caught_pokemon * pokemon = mensaje_crear_caught_pokemon(1);

    t_paquete* paquete = paquete_from_caught_pokemon(pokemon);
    printf("tipo_mensaje:%d\n", paquete->header->tipo_mensaje);
    printf("payload_size:%d\n", paquete->header->payload_size);
    printf("is_caught: %s\n\n", pokemon->is_caught ? "true" : "false");

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
