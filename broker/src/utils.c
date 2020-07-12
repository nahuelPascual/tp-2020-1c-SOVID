/*
 * utils.c
 *
 *  Created on: 9 jul. 2020
 *      Author: utnso
 */

#include "utils.h"

int get_siguiente_potencia_de_2(int numero) {
    int potencia = 1;
    while(potencia < numero)
        potencia*=2;

    return potencia;
}
