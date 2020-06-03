//
// Created by utnso on 13/05/20.
//

#ifndef TEAM_ENTRENADOR_H
#define TEAM_ENTRENADOR_H

#include <stdbool.h>
#include <math.h>
#include <semaphore.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <delibird-commons/model/mensaje.h>
#include "pokemon.h"
#include "sender.h"
#include "configuration.h"

sem_t sem_post_ejecucion;

typedef enum {
    NEW, READY, EXECUTE, BLOCKED_IDLE, BLOCKED_WAITING, BLOCKED_FULL, EXIT
} t_estado;

typedef struct {
    int ciclos_cpu_ejecutados;
    int deadlocks;
    /* datos para SJF */
    float ultima_ejecucion;
    float ultima_estimacion;
    float estimado_siguiente_rafaga;
    int ejecucion_parcial;
} t_info;

typedef struct {
    int id;
    t_list* objetivos;
    t_list* capturados;
    t_estado estado;
    t_coord* posicion;
    t_pokemon_mapeado* objetivo_actual;
    t_info* info;
} t_entrenador;


t_entrenador* entrenador_new(int id, t_list* objetivos, t_list* capturados,t_coord* posicion);
t_entrenador* entrenador_get_libre_mas_cercano(t_coord* posicion_buscada);
void entrenador_init_list(t_list* e);
void entrenador_execute(t_entrenador* e);
void entrenador_otorgar_ciclos_ejecucion(t_entrenador* entrenador, int cant);
t_entrenador* entrenador_get(int id);
t_list* entrenador_get_execute();
t_list* entrenador_get_all();
int entrenador_get_count();
int entrenador_calcular_remaining(t_entrenador* e);
void log_entrenadores(t_list*);

#endif //TEAM_ENTRENADOR_H
