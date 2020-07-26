//
// Created by utnso on 11/05/20.
//

#include "config.h"

t_config* config;

void config_init(char* path) {
    config = config_create(path);
}

void config_close() {
    config_destroy(config);
}

uint32_t get_id_suscriptor() {
    return config_get_int_value(config, "ID_SUSCRIPTOR");
}

char* get_ip(char* proceso) {
    char* str = string_new();
    string_append(&str, "IP_");
    string_append(&str, proceso);
    char* value = config_get_string_value(config, str);
    free(str);
    return value;
}

char* get_puerto(char* proceso) {
    char* str = string_new();
    string_append(&str, "PUERTO_");
    string_append(&str, proceso);
    char* value = config_get_string_value(config, str);
    free(str);
    return value;
}
