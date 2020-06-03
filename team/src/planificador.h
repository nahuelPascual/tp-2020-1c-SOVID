//
// Created by utnso on 12/05/20.
//

#ifndef TEAM_PLANIFICADOR_H
#define TEAM_PLANIFICADOR_H

#include <semaphore.h>
#include <pthread.h>
#include <math.h>
#include <stdbool.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include "entrenador.h"
#include "configuration.h"

pthread_mutex_t mx_cola_ready;

void planificador_init();
void planificador_encolar_ready(t_entrenador* e);

#endif //TEAM_PLANIFICADOR_H
