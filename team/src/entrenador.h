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
    int id_otro_entrenador;
    t_coord* ubicacion;
    char* entrego_pokemon;
    char* recibo_pokemon;
    int remaining_intercambio;
} t_intercambio;

typedef struct {
    int id;
    t_list* objetivos;
    t_list* capturados;
    t_estado estado;
    bool deadlock;
    t_coord* posicion;
    t_pokemon_mapeado* pokemon_buscado;
    t_intercambio* intercambio;
    t_info* info;
} t_entrenador;

t_entrenador* entrenador_new(int id, t_list* objetivos, t_list* capturados,t_coord* posicion);
t_entrenador* entrenador_get_libre_mas_cercano(t_coord* posicion_buscada);
void entrenador_init_list(t_list* entrenadores);
void entrenador_execute(t_entrenador*);
void entrenador_otorgar_ciclos_ejecucion(t_entrenador* entrenador, int cant);
t_entrenador* entrenador_get(int id);
t_list* entrenador_get_execute();
t_list* entrenador_get_all();
int entrenador_get_count();
int entrenador_calcular_remaining(t_entrenador*);
void log_entrenadores(t_list*);
void entrenador_verificar_objetivos(t_entrenador*);
t_list* entrenador_get_bloqueados();
t_list* entrenador_calcular_pokemon_faltantes(t_entrenador*);
t_list* entrenador_calcular_pokemon_sobrantes(t_entrenador*);

#endif //TEAM_ENTRENADOR_H
