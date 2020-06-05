//
// Created by utnso on 12/05/20.
//

#include "planificador.h"

const int COSTO_INTERCAMBIO = 5;

static enum {FIFO, ROUND_ROBIN, SJF, SJF_CON_DESALOJO};
static t_queue* cola_ready;
static sem_t sem_entrenadores_planificables;
static int algoritmo;
static float alpha;

static void planificar();
static t_entrenador* planificar_FIFO();
static t_entrenador* planificar_RR(int quantum);
static t_entrenador* planificar_SJF();
static t_entrenador* planificar_SJF_con_desalojo();
static float estimar_proxima_rafaga(t_entrenador*);
static int normalizar_algoritmo_planificacion(char* algoritmo);
static t_entrenador* elegir_entrenador();
static bool es_SJF();
static t_entrenador* detectar_deadlock(t_entrenador*);
static void verificar_deadlock_exit(t_entrenador*);
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
            entrenador_planificado = planificar_SJF();
            break;
        case SJF_CON_DESALOJO:
            entrenador_planificado = planificar_SJF_con_desalojo();
            break;
        default:
            log_error(default_logger, "Algoritmo de planificacion no definido");
            exit(1);
        }

        sem_wait(&sem_post_ejecucion);

        // quantum consumido o SJF-CD
        if (entrenador_planificado->estado == EXECUTE) {
            planificador_encolar_ready(entrenador_planificado);
            continue;
        }

        // acaba de concretar un intercambio
        if (entrenador_planificado->deadlock) {
            t_entrenador* otro_entrenador = entrenador_get(entrenador_planificado->intercambio->id_otro_entrenador);
            entrenador_planificado->deadlock = otro_entrenador->deadlock = false;
            free(entrenador_planificado->intercambio);
            verificar_deadlock_exit(entrenador_planificado);
            verificar_deadlock_exit(otro_entrenador);
        } else {
            verificar_deadlock_exit(entrenador_planificado);
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

static void verificar_deadlock_exit(t_entrenador* e) {
    if (e->estado == EXIT) {
        bool _cumple_objetivos(void* item) {
            t_entrenador* entrenador = (t_entrenador*) item;
            bool _check(void* _item) {
                t_pokemon_objetivo* objetivo = (t_pokemon_objetivo*) _item;
                return objetivo->fue_capturado;
            }
            return list_all_satisfy(entrenador->objetivos, (void*)_check);
        }
        if (list_all_satisfy(entrenador_get_all(), (void*)_cumple_objetivos)) {
            // TODO cerrar proceso por objetivo global cumplido
        }
    }

    t_entrenador* otro_entrenador = NULL;
    if (e->estado == BLOCKED_FULL && (otro_entrenador = detectar_deadlock(e)) != NULL) {
        e->deadlock = otro_entrenador->deadlock = true;
        e->info->deadlocks++;
        otro_entrenador->info->deadlocks++;
        planificador_encolar_ready(e);
    }
}

static t_entrenador* detectar_deadlock(t_entrenador* entrenador) {
    if (entrenador->deadlock) {
        // cuando vuelvo de resolver un deadlock entre Entrenador1 y Entrenador2,
        // puede darse que Entrenador1 se planifique para resolver un nuevo deadlock con Entrenador2
        // y luego Entrenador2 quiera planificarse para resolver deadlock con Entrenador3.
        return NULL;
    }

    t_list* bloqueados = entrenador_get_bloqueados();
    t_list* faltantes = entrenador_calcular_pokemon_faltantes(entrenador);
    t_list* mis_sobrantes = entrenador_calcular_pokemon_sobrantes(entrenador);

    bool _es_mi_sobrante(void* item) {
        t_pokemon_objetivo* faltante = (t_pokemon_objetivo*) item;
        bool _equals(void* item) {
            t_pokemon_objetivo* mi_sobrante = (t_pokemon_objetivo*) item;
            return string_equals_ignore_case(faltante->nombre, mi_sobrante->nombre);
        }
        return list_any_satisfy(mis_sobrantes, (void*)_equals);
    }
    t_entrenador* _es_mi_faltante(void* item) {
        t_pokemon_objetivo* sobrante = (t_pokemon_objetivo*) item;
        bool _equals(void* item) {
            t_pokemon_objetivo* faltante = (t_pokemon_objetivo*) item;
            return string_equals_ignore_case(faltante->nombre, sobrante->nombre);
        }
        return list_find(faltantes, (void*)_equals);
    }

    for (int i = 0 ; i < list_size(bloqueados) ; i++) {
        t_entrenador* un_bloqueado = list_get(bloqueados, i);
        t_list* faltantes_otro = entrenador_calcular_pokemon_faltantes(un_bloqueado);
        t_list* sobrantes_otro = entrenador_calcular_pokemon_sobrantes(un_bloqueado);

        t_pokemon_objetivo* entrego = (t_pokemon_objetivo*) list_find(faltantes_otro, (void*)_es_mi_sobrante);
        t_pokemon_objetivo* recibo = (t_pokemon_objetivo*) list_find(sobrantes_otro, (void*)_es_mi_faltante);
        if (entrego != NULL && recibo != NULL) {
            t_intercambio* intercambio = malloc(sizeof(intercambio));
            intercambio->entrego_pokemon = entrego->nombre;
            intercambio->recibo_pokemon = recibo->nombre;
            intercambio->ubicacion = un_bloqueado->posicion;
            intercambio->remaining_intercambio = COSTO_INTERCAMBIO;
            entrenador->intercambio = intercambio;
            return un_bloqueado;
        }
    }

    return NULL;
}

static t_entrenador* planificar_FIFO() {
    t_entrenador* e = queue_peek(cola_ready);
    int remaining = entrenador_calcular_remaining(e);
    planificar_RR(remaining);
    return e;
}

static t_entrenador* planificar_RR(int quantum) {
    t_entrenador* entrenador = queue_pop(cola_ready);
    entrenador_otorgar_ciclos_ejecucion(entrenador, quantum);
    return entrenador;
}

static t_entrenador* planificar_SJF() {
    t_entrenador* entrenador = elegir_entrenador();
    planificar_FIFO();
    return entrenador;
}

static t_entrenador* planificar_SJF_con_desalojo() {
    t_entrenador* entrenador = elegir_entrenador();
    planificar_RR(1);
    return entrenador;
}

static t_entrenador* elegir_entrenador() {
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
