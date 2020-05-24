//
// Created by utnso on 13/05/20.
//

#ifndef TEAM_ENTRENADOR_H
#define TEAM_ENTRENADOR_H

#include <stdbool.h>
#include <math.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <delibird-commons/model/mensaje.h>
#include <pthread.h>
#include "planificador.h"
#include "utils/parser.h"

typedef enum {
    NEW, READY, EXECUTE, BLOCKED_IDLE, BLOCKED_WAITING, BLOCKED_FULL, EXIT
} t_estado;

typedef struct {
    int thread_id;
    t_list* objetivos;
    t_list* pokemon_atrapados;
    t_estado estado;
    t_coord* posicion;
} t_entrenador;

t_entrenador* entrenador_get_libre_mas_cercano(t_coord* posicion_buscada);
void entrenador_init_list(t_list* e);
void entrenador_set_ready(t_entrenador* e);
void entrenador_execute(t_entrenador* e);
t_list* entrenador_all();
void log_entrenadores(t_list*);

#endif //TEAM_ENTRENADOR_H
