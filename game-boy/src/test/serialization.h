//
// Created by utnso on 17/04/20.
//

#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <commons/collections/list.h>
#include <delibird-commons/utils/ipc.h>

void test_new_pokemon(int conn);
void test_localized_pokemon(int conn);
void test_get_pokemon(int conn);
void test_appeared_pokemon(int conn);
void test_catch_pokemon(int conn);
void test_caught_pokemon(int conn);
void test_ack(int conn);
void test_suscripcion(int conn);
void test_informe_id(int conn);
void test_serializarEnviarTodos(char* ip, char* puerto);

#endif //SERIALIZATION_H
