//
// Created by utnso on 16/05/20.
//

#ifndef LOGGER_H
#define LOGGER_H

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <commons/log.h>
#include "paquete.h"

void logger_conexion(t_log* logger, char* proceso, char* ip, char* puerto);

void logger_paquete(t_paquete* paquete, t_log* logger);

void logger_mensaje(t_log* logger, t_paquete* paquete);
void logger_ack(t_log* logger, t_ack* ack);
void logger_suscripcion(t_log* logger, t_suscripcion* suscripcion);
void logger_informe(t_log* logger, t_informe_id* informe_id);

void logger_new_pokemon(t_log* logger, t_new_pokemon* new_pokemon);
void logger_localized_pokemon(t_log* logger, t_localized_pokemon* localized_pokemon);
void logger_get_pokemon(t_log* logger, t_get_pokemon* get_pokemon);
void logger_appeared_pokemon(t_log* logger, t_appeared_pokemon* appeared_pokemon);
void logger_catch_pokemon(t_log* logger, t_catch_pokemon* catch_pokemon);
void logger_caught_pokemon(t_log* logger, t_caught_pokemon* caught_pokemon);

void logger_recibido(t_log* logger, t_paquete* paquete);
void logger_enviado(t_log* logger, t_paquete* paquete);
void logger_enviado_a_un_suscriptor(t_log* logger, t_paquete* paquete, uint32_t id_suscriptor);

#endif //LOGGER_H
