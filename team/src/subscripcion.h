//
// Created by utnso on 9/05/20.
//

#ifndef TEAM_SUBSCRIPCION_H
#define TEAM_SUBSCRIPCION_H

#include <stdio.h>
#include <stdbool.h>
#include <delibird-commons/utils/ipc.h>
#include <delibird-commons/utils/paquete.h>
#include <delibird-commons/model/mensaje.h>
#include <pthread.h>
#include "sender.h"
#include "entrenador.h"
#include "pokemon.h"
#include "utils/objetivos.h"
#include "planificador.h"

void suscribirseAlBroker();
void escuchar(int con);

#endif //TEAM_SUBSCRIPCION_H
