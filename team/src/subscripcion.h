//
// Created by utnso on 9/05/20.
//

#ifndef TEAM_SUBSCRIPCION_H
#define TEAM_SUBSCRIPCION_H

#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <delibird-commons/utils/ipc.h>
#include <delibird-commons/utils/paquete.h>
#include <delibird-commons/model/mensaje.h>
#include <delibird-commons/utils/logger.h>
#include "sender.h"
#include "entrenador.h"
#include "pokemon.h"
#include "objetivos.h"
#include "configuration.h"
#include "planificador.h"

void escuchar_a(int);

#endif //TEAM_SUBSCRIPCION_H
