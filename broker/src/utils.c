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
        potencia *= 2;

    return potencia;
}

char* get_string_datetime() {
    time_t epoch;
    struct tm* info_datetime;
    char* string_datetime = malloc(80);

    time(&epoch);
    info_datetime = localtime(&epoch);
    strftime(string_datetime, 80, "%d/%m/%Y %X", info_datetime);

    return string_datetime;
}
