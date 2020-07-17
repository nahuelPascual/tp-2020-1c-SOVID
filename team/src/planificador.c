//
// Created by utnso on 12/05/20.
//

#include "planificador.h"

extern t_log* default_logger;

static enum {FIFO, ROUND_ROBIN, SJF, SJF_CON_DESALOJO};
static t_queue* cola_ready;
static pthread_mutex_t mx_cola_ready;
static sem_t sem_entrenadores_planificables;
static int algoritmo;
static float alpha;

static void planificar();
static t_entrenador* planificar_FIFO();
static t_entrenador* planificar_RR(int quantum);
static t_entrenador* planificar_SJF();
static float estimar_proxima_rafaga(t_entrenador*);
static int normalizar_algoritmo_planificacion(char* algoritmo);
static t_entrenador* elegir_entrenador();
static bool es_SJF();
static t_entrenador* pop();
static void push(t_entrenador*);

pthread_t planificador_init() {
    sem_init(&sem_entrenadores_planificables, 0, 0);
    algoritmo = normalizar_algoritmo_planificacion(config_team->algoritmo_planificacion);
    cola_ready = queue_create();
    pthread_mutex_init(&mx_cola_ready, NULL);
    alpha = config_team->alpha;

    pthread_t planificador;
    pthread_create(&planificador, NULL, (void*)planificar, NULL);
    return planificador;
}

static void planificar(){
    while (1) {
        t_entrenador* entrenador_planificado = NULL;

        sem_wait(&sem_entrenadores_planificables);
        switch(algoritmo) {
        case FIFO:
            entrenador_planificado = planificar_FIFO();
            break;
        case ROUND_ROBIN:
            entrenador_planificado = planificar_RR(config_team->quantum);
            break;
        case SJF:
        case SJF_CON_DESALOJO:
            entrenador_planificado = planificar_SJF();
            break;
        default:
            log_error(default_logger, "Algoritmo de planificacion no definido");
            exit(1);
        }

        metricas_add_cambio_contexto(); // inicia proceso
        sem_wait(&sem_post_ejecucion);
        metricas_add_cambio_contexto(); // vuelve planificador

        switch (entrenador_planificado->estado) {
            case EXECUTE: // quantum consumido o SJF-CD
                planificador_encolar_ready(entrenador_planificado);
                break;
            case BLOCKED_IDLE: // acaba de capturar automaticamente al enviar catch
                planificador_admitir(entrenador_planificado);
                break;
            case BLOCKED_WAITING:
                break;
            case EXIT: case BLOCKED_FULL:
                // acaba de concretar un intercambio
                if (entrenador_planificado->deadlock) {
                    t_entrenador* otro_entrenador = entrenador_get(entrenador_planificado->intercambio->id_otro_entrenador);
                    entrenador_planificado->deadlock = false;
                    if (otro_entrenador->intercambio == NULL) otro_entrenador->deadlock = false;
                    else otro_entrenador->intercambio->entrego_pokemon = entrenador_planificado->intercambio->entrego_pokemon;
                    free(entrenador_planificado->intercambio); // solo libero t_intercambio porque pokemons y ubicacion pertenecen a los entrenadores
                    entrenador_planificado->intercambio = NULL;
                    planificador_verificar_deadlock_exit(entrenador_planificado);
                    planificador_verificar_deadlock_exit(otro_entrenador);
                } else {
                    planificador_verificar_deadlock_exit(entrenador_planificado);
                }
                break;
            default:
                log_error(default_logger, "Ejecuto un entrenador con estado invalido: %s", string_itoa(entrenador_planificado->estado));
        }
    }
}

void planificador_encolar_ready(t_entrenador* e) {
    if (es_SJF()) { // no se deben actualizar los datos de estimacion cuando SJF hace una ejecucion parcial
        if (algoritmo == SJF_CON_DESALOJO) entrenador_abortar_ejecucion();
        if (e->estado != EXECUTE) {
            e->info->ultima_estimacion = e->info->estimado_siguiente_rafaga;
            e->info->ultima_ejecucion = e->info->ejecucion_parcial;
            e->info->ejecucion_parcial = 0;
        }
        e->info->estimado_siguiente_rafaga = e->info->ciclos_cpu_ejecutados == 0 ? config_team->estimacion_inicial : estimar_proxima_rafaga(e); // recalcula con mismos datos pero con ejecucion parcial actualizada
    }
    logs_transicion(e, READY);
    e->estado = READY;
    push(e);
    sem_post(&sem_entrenadores_planificables);
}

void planificador_verificar_deadlock_exit(t_entrenador* e) {
    if (e->estado == EXIT && list_all_satisfy(entrenador_get_all(), (void*)entrenador_cumplio_objetivos)) {
        log_debug(default_logger, "Todos los objetivos del team fueron cumplidos!");
        metricas_calcular();
        pthread_exit(0);
    }

    t_entrenador* otro_entrenador = NULL;
    pthread_mutex_lock(&mx_entrenadores);
    if (e->estado == BLOCKED_FULL && (otro_entrenador = deadlock_detectar(e)) != NULL) {
        log_debug(default_logger, "Se planifica al entrenador #%d hacia la posicion (%d, %d) para resolver DEADLOCK con entrenador #%d",
                e->id, otro_entrenador->posicion->x, otro_entrenador->posicion->y, otro_entrenador->id);
        planificador_encolar_ready(e);
    }
    pthread_mutex_unlock(&mx_entrenadores);
}

void planificador_reasignar(char* pokemon) {
    log_debug(default_logger, "Se intenta replanificar la captura de %s", pokemon);
    pthread_mutex_lock(&mx_entrenadores);
    pthread_mutex_lock(&mx_pokemon);
    t_entrenador* entrenador = entrenador_asignar(pokemon);
    pthread_mutex_unlock(&mx_pokemon);
    if (entrenador == NULL) return;

    log_debug(default_logger, "Se planifica al entrenador #%d en (%d, %d) hacia la posicion (%d, %d)",
            entrenador->id, entrenador->posicion->x, entrenador->posicion->y,
            entrenador->pokemon_buscado->ubicacion->x, entrenador->pokemon_buscado->ubicacion->y);

    planificador_encolar_ready(entrenador);
    pthread_mutex_unlock(&mx_entrenadores);
}

void planificador_admitir(t_entrenador* e) {
    pthread_mutex_lock(&mx_entrenadores);
    if (e->estado != BLOCKED_IDLE) {
        pthread_mutex_unlock(&mx_entrenadores);
        return;
    }
    entrenador_asignar_objetivo_a(e);
    if(e->pokemon_buscado == NULL) {
        log_debug(default_logger, "No se replanifica al entrenador #%d porque no hay mas objetivos disponibles", e->id);
        pthread_mutex_unlock(&mx_entrenadores);
        return;
    }
    log_debug(default_logger, "Se replanifico al entrenador #%d para capturar un %s en la posicion (%d, %d)",
            e->id, e->pokemon_buscado->pokemon, e->pokemon_buscado->ubicacion->x, e->pokemon_buscado->ubicacion->y);
    planificador_encolar_ready(e);
    pthread_mutex_unlock(&mx_entrenadores);
}

static t_entrenador* pop() {
    pthread_mutex_lock(&mx_cola_ready);
    t_entrenador* e = queue_pop(cola_ready);
    pthread_mutex_unlock(&mx_cola_ready);
    return e;
}

static void push(t_entrenador* e) {
    pthread_mutex_lock(&mx_cola_ready);
    queue_push(cola_ready, e);
    pthread_mutex_unlock(&mx_cola_ready);
}

static t_entrenador* planificar_FIFO() {
    t_entrenador* e = queue_peek(cola_ready);
    int remaining = entrenador_calcular_remaining(e);
    planificar_RR(remaining);
    return e;
}

static t_entrenador* planificar_RR(int quantum) {
    t_entrenador* entrenador = pop();
    log_debug(default_logger, "Se ejecuta el entrenador #%d", entrenador->id);
    entrenador_otorgar_ciclos_ejecucion(entrenador, quantum);
    return entrenador;
}

static t_entrenador* planificar_SJF() {
    t_entrenador* entrenador = elegir_entrenador();
    planificar_FIFO();
    return entrenador;
}

static t_entrenador* elegir_entrenador() {
    pthread_mutex_lock(&mx_cola_ready);
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
    t_entrenador* entrenador = list_remove(entrenadores, indice);
    list_add_in_index(entrenadores, 0, entrenador);
    pthread_mutex_unlock(&mx_cola_ready);
    return entrenador;
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
