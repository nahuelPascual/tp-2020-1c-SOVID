/*
 * buzon.c
 *
 *  Created on: 18 jun. 2020
 *      Author: utnso
 */

#include "buzon.h"

t_buzon* buzon_crear(
    int tamanio_memoria,
    int tamanio_minimo_particion,
    t_algoritmo_memoria algoritmo_memoria,
    t_algoritmo_reemplazo algoritmo_reemplazo,
    t_algoritmo_particion_libre algoritmo_particion_libre,
    int frecuencia_compactacion
) {
    t_buzon* buzon = malloc(sizeof(t_buzon));
    buzon->administrador_colas = administrador_colas_crear();
    buzon->memoria = memoria_crear(
        tamanio_memoria,
        tamanio_minimo_particion,
        algoritmo_memoria,
        algoritmo_particion_libre,
        algoritmo_reemplazo,
        frecuencia_compactacion
    );
    pthread_mutex_init(&buzon->mutex_memoria, NULL);

    return buzon;
}

t_mensaje_despachable* buzon_almacenar_mensaje(t_buzon* buzon, t_paquete* paquete) {
    t_mensaje_despachable* mensaje_despachable = NULL;
    t_cola* cola = administrador_colas_get_cola_from(buzon->administrador_colas, paquete->header->tipo_mensaje);

    if(!cola_es_mensaje_redundante(cola, paquete)) {
        pthread_mutex_lock(&buzon->mutex_memoria);
        buzon_vaciar_hasta_tener(buzon, paquete->header->payload_size);

        mensaje_despachable = mensaje_despachable_from_paquete(paquete, buzon->memoria);

        administrador_colas_asignar_id_mensaje_a(buzon->administrador_colas, mensaje_despachable);

        memoria_dividir_particion_si_es_mas_grande_que(buzon->memoria, mensaje_despachable->particion_asociada, mensaje_despachable->size);

        memoria_asignar_paquete_a_la_particion(buzon->memoria, paquete, mensaje_despachable->particion_asociada);

        particion_ocupar(mensaje_despachable->particion_asociada);

        logger_mensaje_almacenado(mensaje_despachable);
        logger_detalle_memoria(buzon->memoria);
        logger_espacio_ocupado(buzon->memoria);
        logger_espacio_libre(buzon->memoria);
        pthread_mutex_unlock(&buzon->mutex_memoria);

        cola_push_mensaje_sin_despachar(cola, mensaje_despachable);
    }

    return mensaje_despachable;
}

void buzon_despachar_mensaje_de(t_buzon* buzon, t_cola* cola) {
    t_mensaje_despachable* mensaje_despachable = cola_pop_mensaje_sin_despachar(cola);

    void _despachar_mensaje_a(t_suscriptor* suscriptor) {
        pthread_mutex_lock(&buzon->mutex_memoria);
        t_paquete* paquete = mensaje_despachable_to_paquete(mensaje_despachable, buzon->memoria);
        pthread_mutex_unlock(&buzon->mutex_memoria);

        paquete_set_tipo_mensaje(paquete, cola->tipo_mensaje);
        bool enviado = ipc_enviar_a(suscriptor->socket_asociado, paquete);

        if(enviado) {
            mensaje_despachable_add_suscriptor_enviado(mensaje_despachable, suscriptor);

            logger_mensaje_enviado(paquete, suscriptor);
        }

        paquete_liberar(paquete);
    }

    cola_iterate_suscriptores(cola, _despachar_mensaje_a);

    cola_push_mensaje_despachado(cola, mensaje_despachable);
}

void buzon_vaciar_hasta_tener(t_buzon* buzon, int espacio) {
    while(!memoria_existe_particion_libre_con(buzon->memoria, espacio)) {
        if(memoria_corresponde_compactar(buzon->memoria)) {
            logger_compactacion_ejecutada();

            memoria_compactar(buzon->memoria);

            logger_detalle_memoria(buzon->memoria);

            memoria_resetear_contador_particiones_desocupadas(buzon->memoria);

            if(memoria_existe_particion_libre_con(buzon->memoria, espacio))
                break;
        }

        t_particion* particion_victima = memoria_get_particion_a_desocupar(buzon->memoria);
        int id_mensaje_victima = particion_victima->id_mensaje_asociado;
        administrador_colas_remove_and_destroy_mensaje_despachable_by_id(buzon->administrador_colas, particion_victima->id_mensaje_asociado);
        particion_desocupar(particion_victima);

        logger_mensaje_eliminado(id_mensaje_victima, particion_victima);
        logger_detalle_memoria(buzon->memoria);

        int cantidad_particiones_consolidadas = memoria_consolidar(buzon->memoria);

        if(cantidad_particiones_consolidadas > 1) {
            logger_detalle_memoria(buzon->memoria);
        }

        memoria_aumentar_contador_particiones_desocupadas(buzon->memoria);
    }
}

void buzon_registrar_suscriptor(t_buzon* buzon, t_suscriptor* suscriptor) {
    t_cola* cola = administrador_colas_get_cola_from(buzon->administrador_colas, suscriptor->tipo_mensaje);

    cola_add_or_update_suscriptor(cola, suscriptor);

    void _enviar_al_suscriptor_si_no_lo_recibio(t_mensaje_despachable* mensaje_despachable) {
        bool fue_recibido = mensaje_despachable_fue_recibido_por(mensaje_despachable, suscriptor);

        if(!fue_recibido) {
            pthread_mutex_lock(&buzon->mutex_memoria);
            t_paquete* paquete = mensaje_despachable_to_paquete(mensaje_despachable, buzon->memoria);
            pthread_mutex_unlock(&buzon->mutex_memoria);

            paquete_set_tipo_mensaje(paquete, cola->tipo_mensaje);
            bool enviado = ipc_enviar_a(suscriptor->socket_asociado, paquete);

            if(enviado) {
                bool fue_enviado_anteriormente = mensaje_despachable_fue_enviado_a(mensaje_despachable, suscriptor);
                if(!fue_enviado_anteriormente)
                    mensaje_despachable_add_suscriptor_enviado(mensaje_despachable, suscriptor);

                logger_mensaje_enviado(paquete, suscriptor);
            }

            paquete_liberar(paquete);
        }
    }

    cola_iterate_mensajes_despachados(cola, _enviar_al_suscriptor_si_no_lo_recibio);
}

void buzon_recibir_ack(t_buzon* buzon, t_ack* ack) {
    t_mensaje_despachable* mensaje_despachable = administrador_colas_find_mensaje_despachable_by_id(buzon->administrador_colas, ack->id_mensaje);

    if(mensaje_despachable) {
        mensaje_despachable_add_suscriptor_recibido(mensaje_despachable, ack);

        if(mensaje_despachable_tiene_todos_los_acks(mensaje_despachable))
            logger_mensaje_tiene_todos_los_acks(mensaje_despachable);
    }
    else {
        logger_mensaje_eliminado_antes_de_recibir_ack(ack);
    }
}

void buzon_imprimir_estado_en(t_buzon* buzon, char* path_archivo) {
    FILE* dump_file = fopen(path_archivo, "w");

    char* string_datetime = get_string_datetime();

    fprintf(dump_file, "Dump: %s\n\n", string_datetime);

    free(string_datetime);

    int i = 0;
    void _write(t_particion* particion) {
        void* desde = memoria_get_direccion_fisica(buzon->memoria, particion->base);
        void* hasta = memoria_get_direccion_fisica(buzon->memoria, particion->base + particion->tamanio - 1);
        char* estado = particion->esta_libre ? "L" : "X";

        char* tamanio_string = string_itoa(particion->tamanio);
        string_append(&tamanio_string,"b");

        int LRU = particion->tiempo_ultima_referencia;

        t_cola* cola = administrador_colas_find_cola_by_id_mensaje(buzon->administrador_colas, particion->id_mensaje_asociado);
        char* tipo_mensaje_string = mensaje_get_tipo_as_string(cola ? cola->tipo_mensaje : NO_APLICA);

        int id = particion->id_mensaje_asociado;

        fprintf(dump_file, "Particion %2i: %p - %p  [%s]  Size: %-5s LRU: %-6i Cola: %-17s ID: %i\n", i, desde, hasta, estado, tamanio_string, LRU, tipo_mensaje_string, id);

        free(tamanio_string);
        i++;
    }

    pthread_mutex_lock(&buzon->mutex_memoria);
    list_iterate(buzon->memoria->particiones, (void*) _write);
    pthread_mutex_unlock(&buzon->mutex_memoria);

    fclose(dump_file);
}
