//
// Created by utnso on 27/06/20.
//

#ifndef TEAM_METRICAS_H
#define TEAM_METRICAS_H

#include "commons/collections/dictionary.h"
#include "commons/collections/list.h"
#include "entrenador.h"

typedef struct {
    int ciclos_cpu_totales;
    t_dictionary* ciclos_entrenador;
    int cambios_contexto_totales;
    int deadlocks_totales;
} t_metricas;

void metricas_add_deadlock();
void metricas_add_cambio_contexto();
void metricas_calcular();

#endif //TEAM_METRICAS_H
