//
// Created by utnso on 17/07/20.
//

#ifndef TEAM_SINCRONIZACION_H
#define TEAM_SINCRONIZACION_H

#include <commons/collections/dictionary.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

pthread_mutex_t mx_semaforos;

void sincronizacion_init();
void sincronizacion_lock(char*);
void sincronizacion_unlock(char*);

#endif //TEAM_SINCRONIZACION_H
