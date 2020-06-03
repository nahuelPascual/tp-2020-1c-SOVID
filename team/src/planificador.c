//
// Created by utnso on 12/05/20.
//

#include "planificador.h"

static enum {FIFO, ROUND_ROBIN, SJF, SJF_CON_DESALOJO};
static t_queue* cola_ready;
static sem_t sem_entrenadores_planificables;
static int algoritmo;
static float alpha;

static void planificar();
static void planificar_FIFO();
static void planificar_RR(int quantum);
static void planificar_SJF();
static void planificar_SJF_con_desalojo();
static float estimar_proxima_rafaga(t_entrenador* e);
static int normalizar_algoritmo_planificacion(char* algoritmo);
static void elegir_entrenador();
static bool es_SJF();
// static t_entrenador* pop(t_queue*); TODO sincronizar queue

void planificador_init() {
    sem_init(&sem_entrenadores_planificables, 0, 0);
    algoritmo = normalizar_algoritmo_planificacion(config_team->algoritmo_planificacion);
    cola_ready = queue_create();
//    pthread_mutex_init(&mx_cola_ready, NULL);
//    alpha = config_team->alpha;
    alpha = 0.5; //TODO levantar de config
    planificar();
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
        case SJF:
            planificar_SJF();
            break;
        case SJF_CON_DESALOJO:
            planificar_SJF_con_desalojo();
            break;
        default:
            log_error(default_logger, "Algoritmo de planificacion no definido");
            exit(1);
        }

        sem_wait(&sem_post_ejecucion);
        t_list* ejecutando = entrenador_get_execute();
        if (list_size(ejecutando) > 1 ) {
            log_error(default_logger, "Grado de multiprocesamiento invalido: %d hilos ejecutando", list_size(ejecutando));
            exit(1);
        }
        if (list_size(ejecutando) == 1) {
            t_entrenador* e = list_get(ejecutando, 0);
            planificador_encolar_ready(e);
        }
    }
}

void planificador_encolar_ready(t_entrenador* e) {
    if (es_SJF()) { // no se deben actualizar los datos de estimacion cuando SJF hace una ejecucion parcial
        if (e->estado != EXECUTE) {
            e->info->ultima_estimacion = e->info->estimado_siguiente_rafaga;
            e->info->ultima_ejecucion = e->info->ejecucion_parcial;
            e->info->ejecucion_parcial = 0;
        }
        e->info->estimado_siguiente_rafaga = estimar_proxima_rafaga(e); // recalcula con mismos datos pero con ejecucion parcial actualizada
    }
    e->estado = READY;
    queue_push(cola_ready, e);
    sem_post(&sem_entrenadores_planificables);
}

static void planificar_FIFO() {
    t_entrenador* e = queue_peek(cola_ready);
    int remaining = entrenador_calcular_remaining(e);
    planificar_RR(remaining);
}

static void planificar_RR(int quantum) {
    entrenador_otorgar_ciclos_ejecucion(queue_pop(cola_ready), quantum);
}

static void planificar_SJF() {
    elegir_entrenador();
    planificar_FIFO();
}

static void planificar_SJF_con_desalojo() {
    elegir_entrenador();
    planificar_RR(1);
}

static void elegir_entrenador() {
    t_list* entrenadores = cola_ready->elements;
    float menor_estimado = 99999;
    int indice;
    for (int i=0; i<list_size(entrenadores); i++) {
        t_entrenador* e = list_get(entrenadores, i);
        if (e->info->estimado_siguiente_rafaga < menor_estimado) {
            menor_estimado = e->info->estimado_siguiente_rafaga;
            indice = i;
        }
    }
    list_add_in_index(entrenadores, 0, list_remove(entrenadores, indice));
}

static float estimar_proxima_rafaga(t_entrenador* e) {
    return e->info->ultima_ejecucion * alpha + e->info->ultima_estimacion * (1-alpha) - e->info->ejecucion_parcial;
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

static bool es_SJF() {
    return algoritmo == SJF || algoritmo == SJF_CON_DESALOJO;
}
