//
// Created by utnso on 12/05/20.
//

#include "planificador.h"

static enum {FIFO, ROUND_ROBIN, SJF, SJF_CON_DESALOJO};
static sem_t sem_entrenadores_planificables;
static int algoritmo;

static void planificar();
static void planificar_FIFO();
static void planificar_RR(int quantum);
static int normalizar_algoritmo_planificacion(char* algoritmo);

void planificador_init() {
    sem_init(&sem_entrenadores_planificables, 0, 0);
    algoritmo = normalizar_algoritmo_planificacion(config_team->algoritmo_planificacion);
    cola_ready = queue_create();
    planificar();
}

void planificador_despertar() {
    sem_post(&sem_entrenadores_planificables);
}

static void planificar(){
    while (1) {
        sem_wait(&sem_entrenadores_planificables);
        switch(algoritmo) {
        case FIFO:
            planificar_FIFO();
            break;
        case ROUND_ROBIN:
            planificar_RR(config_team->quantum);
            break;
        default:
            log_error(default_logger, "Algoritmo de planificacion no definido");
            exit(1);
        }
    }
}

static void planificar_FIFO() {
    t_entrenador* e = queue_peek(cola_ready);
    int remaining = entrenador_calcular_remaining(e, e->objetivo_actual->ubicacion);
    planificar_RR(remaining);
}

static void planificar_RR(int quantum) {
    entrenador_otorgar_ciclos_ejecucion(queue_pop(cola_ready), quantum);
}

static int normalizar_algoritmo_planificacion(char* algoritmo) {
    if (string_equals_ignore_case(algoritmo, "FIFO")) {
        return FIFO;
    }
    if (string_equals_ignore_case(algoritmo, "RR")) {
        return ROUND_ROBIN;
    }
    if (string_equals_ignore_case(algoritmo, "SJF")) {
        return SJF;
    }
    if (string_equals_ignore_case(algoritmo, "SJF-CD")) {
        return SJF_CON_DESALOJO;
    }
    log_error(default_logger, "Algoritmo de planificacion no valido");
    exit(1);
}
