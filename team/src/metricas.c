//
// Created by utnso on 27/06/20.
//

#include "metricas.h"

extern t_log* logger;

static int deadlocks_totales;
static int cambios_contexto_totales;

static void log_metricas(t_metricas*);

void metricas_add_deadlock() {
    deadlocks_totales++;
}

void metricas_add_cambio_contexto() {
    cambios_contexto_totales++;
}

void metricas_calcular() {
    t_metricas* metricas = malloc(sizeof(t_metricas));
    metricas->ciclos_cpu_totales = 0;
    metricas->ciclos_entrenador = dictionary_create();

    void _calculate(void* entrenador) {
        t_entrenador* e = (t_entrenador*) entrenador;
        metricas->ciclos_cpu_totales += e->info->ciclos_cpu_ejecutados;
        char* key = string_itoa(e->id);
        dictionary_put(metricas->ciclos_entrenador, key, e->info->ciclos_cpu_ejecutados);
        free(key);
    }
    list_iterate(entrenador_get_all(), (void*)_calculate);

    metricas->cambios_contexto_totales = cambios_contexto_totales;
    metricas->deadlocks_totales = deadlocks_totales;

    log_metricas(metricas);

    dictionary_destroy(metricas->ciclos_entrenador);
    free(metricas);
}

static void log_metricas(t_metricas* m) {
    void _log(char* id, void* ciclos) {
        log_info(logger, "\t\tEntrenador #%s: %d ciclos", id, ciclos);
    }
    log_info(logger, "Metricas team %d:", config_team->id);
    log_info(logger, "\tCiclos CPU ejecutados: %d", m->ciclos_cpu_totales);
    dictionary_iterator(m->ciclos_entrenador, (void*)_log);
    log_info(logger, "\tCambios de contexto realizados: %d", m->cambios_contexto_totales);
    log_info(logger, "\tDeadlocks producidos y resueltos: %d", m->deadlocks_totales);
}
