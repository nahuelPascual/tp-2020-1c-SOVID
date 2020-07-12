/*
 * memoria.c
 *
 *  Created on: 9 jun. 2020
 *      Author: utnso
 */

#include "memoria.h"

t_particion* memoria_crear_particion(int base, int tamanio) {
    t_particion* particion = malloc(sizeof(t_particion));

    particion->esta_libre = true;

    particion->base = base;
    particion->tamanio = tamanio;

    particion->tiempo_carga = 0;
    particion->tiempo_ultima_referencia = 0;

    particion->id_mensaje_asociado = 0;

    return particion;
}

void memoria_liberar_particion(t_particion* particion) {
    free(particion);
}

t_memoria* memoria_crear(
    int tamanio_minimo_particion,
    int tamanio_maximo_memoria,
    t_tipo_esquema_administracion tipo_esquema_administracion,
    t_tipo_algoritmo_ocupado tipo_algoritmo_ocupado,
    t_tipo_algoritmo_desocupado tipo_algoritmo_desocupado,
    int frecuencia_compactacion
) {
    t_memoria* memoria = malloc(sizeof(t_memoria));

    memoria->data = malloc(tamanio_maximo_memoria);

    memoria->tamanio_minimo_particion = tamanio_minimo_particion;
    memoria->tipo_esquema_administracion = tipo_esquema_administracion;
    memoria->tipo_algoritmo_ocupado = tipo_algoritmo_ocupado;
    memoria->tipo_algoritmo_desocupado = tipo_algoritmo_desocupado;
    memoria->frecuencia_compactacion = frecuencia_compactacion;
    memoria->contador_particiones_desocupadas = 0;

    memoria->particiones = list_create();
    list_add(memoria->particiones, memoria_crear_particion(0, tamanio_maximo_memoria));

    return memoria;
}

void* memoria_get_direccion_fisica_de(t_memoria* memoria, t_particion* particion) {
    particion->tiempo_ultima_referencia = clock();

    return memoria->data + particion->base;
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
    int _get_tamanio(t_particion* particion) {
        return particion->tamanio;
    }

    switch(memoria->tipo_algoritmo_ocupado) {
    case FIRST_FIT: {
        return list_find(memoria->particiones, (void*) _esta_libre_y_tiene_el_tamanio_minimo);
    }
    case BEST_FIT: {
        t_list* particiones_candidatas = list_filter(memoria->particiones, (void*) _esta_libre_y_tiene_el_tamanio_minimo);
        t_particion* particion = list_min_by(particiones_candidatas, (void*) _get_tamanio);
        list_destroy(particiones_candidatas);

        return particion;
    }
    default:
        return NULL;
    }
}

void memoria_dividir_particion_si_es_mas_grande_que(t_memoria* memoria, t_particion* particion, int tamanio) {
    int potencia_de_2_si_corresponde = memoria->tipo_esquema_administracion == BUDDY_SYSTEM ? get_siguiente_potencia_de_2(tamanio) : tamanio;
    int tamanio_minimo = fmax(potencia_de_2_si_corresponde, memoria->tamanio_minimo_particion);

    while(particion->tamanio > tamanio_minimo) {
        int indice = list_index_of(memoria->particiones, particion);

        int tamanio_correcto = memoria->tipo_esquema_administracion == BUDDY_SYSTEM ? particion->tamanio / 2 : tamanio_minimo;

        t_particion* nueva_particion = memoria_crear_particion(particion->base + tamanio_correcto, particion->tamanio - tamanio_correcto);

        list_add_in_index(memoria->particiones, indice + 1, nueva_particion);

        particion->tamanio = tamanio_correcto;
    }
}

void memoria_asignar_paquete_a_la_particion(t_memoria* memoria, t_paquete* paquete, t_particion* particion) {
    void* direccion_fisica = memoria_get_direccion_fisica_de(memoria, particion);

    memcpy(direccion_fisica, paquete->payload, paquete->header->payload_size);

    particion->id_mensaje_asociado = paquete->header->id_mensaje;
    particion->esta_libre = false;
    particion->tiempo_carga = clock();
}

t_particion* memoria_get_particion_a_desocupar(t_memoria* memoria) {
    bool _esta_ocupada(t_particion* particion) {
        return !particion->esta_libre;
    }
    int _get_tiempo_carga(t_particion* particion) {
        return particion->tiempo_carga;
    }
    int _get_tiempo_ultima_referencia(t_particion* particion) {
        return particion->tiempo_ultima_referencia;
    }

    t_list* particiones_candidatas = list_filter(memoria->particiones, (void*) _esta_ocupada);
    t_particion* particion = NULL;

    switch(memoria->tipo_algoritmo_desocupado) {
    case FIFO: {
        particion = list_min_by(particiones_candidatas, (void*) _get_tiempo_carga);
        break;
    }
    case LRU: {
        particion = list_min_by(particiones_candidatas, (void*) _get_tiempo_ultima_referencia);
        break;
    }
    default:
        break;
    }

    list_destroy(particiones_candidatas);
    return particion;
}

void memoria_desocupar_particion(t_memoria* memoria, t_particion* particion) {
    particion->esta_libre = true;

    memoria_consolidar(memoria);
}

void memoria_aumentar_contador_particiones_desocupadas(t_memoria* memoria) {
    memoria->contador_particiones_desocupadas++;
}

void memoria_resetear_contador_particiones_desocupadas(t_memoria* memoria){
    memoria->contador_particiones_desocupadas = 0;
}

bool memoria_corresponde_compactar(t_memoria* memoria) {
    return memoria->tipo_esquema_administracion != BUDDY_SYSTEM
    && memoria->frecuencia_compactacion == memoria->contador_particiones_desocupadas;
}

void memoria_compactar(t_memoria* memoria) {
    bool _primero_los_ocupados_y_despues_base_menor(t_particion* una_particion, t_particion* otra_particion) {
        return una_particion->esta_libre == otra_particion->esta_libre ? una_particion->base < otra_particion->base : !una_particion->esta_libre;
    }
    
    list_sort(memoria->particiones, (void*) _primero_los_ocupados_y_despues_base_menor);

    for(int i = 0; i < list_size(memoria->particiones); i++) {
        t_particion* particion = list_get(memoria->particiones, i);
        
        //TODO: Esta es la logica del get_direccion_fisica pero sin actualizar TUR, tenemos repeticion de logica
        void* ubicacion_anterior_info = memoria->data + particion->base;

        if(i == 0) {
            particion->base = 0;
        }
        else {
            t_particion* particion_anterior = list_get(memoria->particiones, i - 1);
            particion->base = particion_anterior->base + particion_anterior->tamanio;
        }

        void* ubicacion_nueva_info = memoria->data + particion->base;
        memmove(ubicacion_nueva_info, ubicacion_anterior_info, particion->tamanio);
    }

    memoria_consolidar(memoria);
}

void memoria_consolidar(t_memoria* memoria) {
    int i = 0;
    while(i < list_size(memoria->particiones) - 1) {
        t_particion* particion = list_get(memoria->particiones, i);
        t_particion* particion_siguiente = list_get(memoria->particiones, i + 1);

        bool particiones_dinamicas_o_son_buddies = memoria->tipo_esquema_administracion == BUDDY_SYSTEM
            ? memoria_son_particiones_buddies(particion, particion_siguiente)
            : true;

        if(particion->esta_libre && particion_siguiente->esta_libre && particiones_dinamicas_o_son_buddies) {
            particion->tamanio += particion_siguiente->tamanio;
            list_remove_and_destroy_element(memoria->particiones, i + 1, (void*) memoria_liberar_particion);

            if(memoria->tipo_esquema_administracion == BUDDY_SYSTEM)
                i = 0;
        }
        else
            i++;
    }
}

bool memoria_son_particiones_buddies(t_particion* una_particion, t_particion* otra_particion) {
    return una_particion->tamanio == otra_particion->tamanio
    && una_particion->base == (otra_particion->base ^ una_particion->tamanio)
    && otra_particion->base == (una_particion->base ^ otra_particion->tamanio);
}
