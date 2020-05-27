//
// Created by utnso on 12/05/20.
//

#ifndef TEAM_PLANIFICADOR_H
#define TEAM_PLANIFICADOR_H

#include <semaphore.h>
#include <pthread.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include "entrenador.h"

t_queue* cola_ready; // se usaria solo para FIFO porque para los otros algoritmos es mas util una lista (supongo)

void planificador_init();
void planificador_despertar();

#endif //TEAM_PLANIFICADOR_H
