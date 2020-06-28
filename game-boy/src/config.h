//
// Created by utnso on 11/05/20.
//

#ifndef TEAM_CONFIG_H
#define TEAM_CONFIG_H

#include <stdint.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/config.h>

void config_init(char* path);
void config_close();
uint32_t get_id_suscriptor();
char* get_ip(char*);
char* get_puerto(char*);

#endif //TEAM_CONFIG_H
