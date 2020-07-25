/*
 * memoria.c
 *
 *  Created on: 9 jun. 2020
 *      Author: utnso
 */

#include "memoria.h"

t_memoria* memoria_crear(
    int tamanio_memoria,
    int tamanio_minimo_particion,
    t_algoritmo_memoria algoritmo_memoria,
    t_algoritmo_reemplazo algoritmo_reemplazo,
    t_algoritmo_particion_libre algoritmo_particion_libre,
    int frecuencia_compactacion
) {
    t_memoria* memoria = malloc(sizeof(t_memoria));

    memoria->data = malloc(tamanio_memoria);

    memoria->tamanio_minimo_particion = tamanio_minimo_particion;
    memoria->algoritmo_memoria = algoritmo_memoria;
    memoria->algoritmo_particion_libre = algoritmo_particion_libre;
    memoria->algoritmo_reemplazo = algoritmo_reemplazo;
    memoria->frecuencia_compactacion = frecuencia_compactacion;
    memoria->contador_particiones_desocupadas = 0;

    memoria->particiones = list_create();
    list_add(memoria->particiones, particion_crear(0, tamanio_memoria));

    return memoria;
}

void* memoria_get_direccion_fisica(t_memoria* memoria, int direccion_logica) {
    return memoria->data + direccion_logica;
}

bool memoria_existe_particion_libre_con(t_memoria* memoria, int tamanio) {
    bool _esta_libre_y_tiene_el_tamanio_minimo(t_particion* particion) {
        int tamanio_minimo = fmax(tamanio, memoria->tamanio_minimo_particion);
        return particion->esta_libre && particion->tamanio >= tamanio_minimo;
    }

    return list_any_satisfy(memoria->particiones, (void*) _esta_libre_y_tiene_el_tamanio_minimo);
}

t_particion* memoria_buscar_particion_libre_con(t_memoria* memoria, int tamanio) {
    bool _esta_libre_y_tiene_el_tamanio_minimo(t_particion* particion) {
        int tamanio_minimo = fmax(tamanio, memoria->tamanio_minimo_particion);
        return particion->esta_libre && particion->tamanio >= tamanio_minimo;
    }

    switch(memoria->algoritmo_particion_libre) {
    case FIRST_FIT: {
        return list_find(memoria->particiones, (void*) _esta_libre_y_tiene_el_tamanio_minimo);
    }
    case BEST_FIT: {
        t_list* particiones_candidatas = list_filter(memoria->particiones, (void*) _esta_libre_y_tiene_el_tamanio_minimo);
        t_particion* particion = list_min_by(particiones_candidatas, (void*) particion_get_tamanio);
        list_destroy(particiones_candidatas);

        return particion;
    }
    default:
        return NULL;
    }
}

void memoria_dividir_particion_si_es_mas_grande_que(t_memoria* memoria, t_particion* particion, int tamanio) {
    int potencia_de_2_si_corresponde = memoria->algoritmo_memoria == BUDDY_SYSTEM ? get_siguiente_potencia_de_2(tamanio) : tamanio;
    int tamanio_minimo = fmax(potencia_de_2_si_corresponde, memoria->tamanio_minimo_particion);

    while(particion->tamanio > tamanio_minimo) {
        int indice = list_index_of(memoria->particiones, particion);

        int tamanio_correcto = memoria->algoritmo_memoria == BUDDY_SYSTEM ? particion->tamanio / 2 : tamanio_minimo;

        t_particion* nueva_particion = particion_crear(particion->base + tamanio_correcto, particion->tamanio - tamanio_correcto);

        list_add_in_index(memoria->particiones, indice + 1, nueva_particion);

        particion->tamanio = tamanio_correcto;
    }
}

void memoria_asignar_paquete_a_la_particion(t_memoria* memoria, t_paquete* paquete, t_particion* particion) {
    void* direccion_fisica = memoria_get_direccion_fisica(memoria, particion->base);

    memcpy(direccion_fisica, paquete->payload, paquete->header->payload_size);
}

t_particion* memoria_get_particion_a_desocupar(t_memoria* memoria) {
    t_list* particiones_candidatas = list_filter(memoria->particiones, (void*) particion_esta_ocupada);
    t_particion* particion = NULL;

    switch(memoria->algoritmo_reemplazo) {
    case FIFO: {
        particion = list_min_by(particiones_candidatas, (void*) particion_get_tiempo_carga);
        break;
    }
    case LRU: {
        particion = list_min_by(particiones_candidatas, (void*) particion_get_tiempo_ultima_referencia);
        break;
    }
    default:
        break;
    }

    list_destroy(particiones_candidatas);
    return particion;
}

void memoria_aumentar_contador_particiones_desocupadas(t_memoria* memoria) {
    memoria->contador_particiones_desocupadas++;
}

void memoria_resetear_contador_particiones_desocupadas(t_memoria* memoria){
    memoria->contador_particiones_desocupadas = 0;
}

bool memoria_corresponde_compactar(t_memoria* memoria) {
    return memoria->algoritmo_memoria != BUDDY_SYSTEM
    && memoria->frecuencia_compactacion == memoria->contador_particiones_desocupadas;
}

void memoria_compactar(t_memoria* memoria) {
    bool _primero_ocupadas_despues_base_menor(t_particion* una_particion, t_particion* otra_particion) {
        return una_particion->esta_libre == otra_particion->esta_libre ? una_particion->base < otra_particion->base : !una_particion->esta_libre;
    }

    list_sort(memoria->particiones, (void*) _primero_ocupadas_despues_base_menor);

    for(int i = 0; i < list_size(memoria->particiones); i++) {
        t_particion* particion = list_get(memoria->particiones, i);
        void* ubicacion_anterior_info = memoria_get_direccion_fisica(memoria, particion->base);

        if(i == 0) {
            particion->base = 0;
        }
        else {
            t_particion* particion_anterior = list_get(memoria->particiones, i - 1);
            particion->base = particion_anterior->base + particion_anterior->tamanio;
        }

        void* ubicacion_nueva_info = memoria_get_direccion_fisica(memoria, particion->base);
        memmove(ubicacion_nueva_info, ubicacion_anterior_info, particion->tamanio);
    }

    logger_detalle_memoria(memoria);

    memoria_consolidar(memoria);
}

int memoria_consolidar(t_memoria* memoria) {
    int i = 0;
    int cantidad_particiones_consolidadas = 1;

    while(i < list_size(memoria->particiones) - 1) {
        t_particion* particion = list_get(memoria->particiones, i);
        t_particion* particion_siguiente = list_get(memoria->particiones, i + 1);

        bool particiones_dinamicas_o_son_buddies = memoria->algoritmo_memoria == BUDDY_SYSTEM
            ? particion_es_buddy_de(particion, particion_siguiente)
            : true;

        if(particion->esta_libre && particion_siguiente->esta_libre && particiones_dinamicas_o_son_buddies) {
            logger_particiones_consolidadas(memoria, particion, particion_siguiente, i, i + 1);

            particion->tamanio += particion_siguiente->tamanio;

            cantidad_particiones_consolidadas++;

            list_remove_and_destroy_element(memoria->particiones, i + 1, (void*) particion_liberar);

            if(memoria->algoritmo_memoria == BUDDY_SYSTEM)
                i = 0;
        }
        else
            i++;
    }

    return cantidad_particiones_consolidadas;
}
