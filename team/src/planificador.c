//
// Created by utnso on 12/05/20.
//

#include "planificador.h"

static enum {FIFO, ROUND_ROBIN, SJF};
static sem_t sem_entrenadores_planificables;
static int algoritmo;

static void planificar_FIFO();

void planificador_init() {
    sem_init(&sem_entrenadores_planificables, 0, 0);
//    TODO algoritmo = config->algoritmo;
    algoritmo = FIFO;
    cola_ready = queue_create();
}

void planificador_despertar() {
    sem_post(&sem_entrenadores_planificables);
}

void planificar(){
    while (1) {
        sem_wait(&sem_entrenadores_planificables);
        t_list* entrenadores_ready = entrenador_filtrar_ready();
        switch(algoritmo) {
        case FIFO:
            planificar_FIFO();
            break;
        default:
            puts("Algoritmo no definido");
            exit(1);
        }
    }
}

static void planificar_FIFO() {
    entrenador_execute(queue_pop(cola_ready));
}
