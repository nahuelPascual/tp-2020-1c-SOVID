/*
 * logger.c
 *
 *  Created on: 22 jul. 2020
 *      Author: utnso
 */

#include "buzon.h"
#include "logger.h"

void logger_iniciando_broker(int tamanio_memoria) {
    log_debug(logger_debug, "Iniciando BROKER con %ib de memoria", tamanio_memoria);
}

void logger_iniciando_despacho_de_mensajes_de(t_tipo_mensaje tipo_mensaje) {
    log_debug(logger_debug, "Iniciando despacho de mensajes de la cola: %s", mensaje_get_tipo_as_string(tipo_mensaje));
}

void logger_iniciando_escucha_de_clientes() {
    log_debug(logger_debug, "Esperando por nuevos clientes");
}

void logger_conexion_iniciada(int cliente) {
    log_info(logger, "< CONEXION INICIADA >", cliente);
}

void logger_suscripcion_recibida(t_suscripcion* suscripcion) {
    log_info(logger, "\tSUSCRIPCION RECIBIDA { cola: %s | suscriptor: %i }",
            mensaje_get_tipo_as_string(suscripcion->tipo_mensaje),
            suscripcion->id_suscriptor);
}

void logger_ack_recibido(t_ack* ack) {
    log_info(logger, "\tACK RECIBIDO { mensaje: %i | suscriptor: %i }", ack->id_mensaje, ack->id_suscriptor);
}

void logger_mensaje_recibido(t_paquete* paquete) {
    log_info(logger, "\t%s RECIBIDO { id: AUN_NO_ASIGNADO | correlation_id: %i | size: %i }",
            mensaje_get_tipo_as_string(paquete->header->tipo_mensaje),
            paquete->header->correlation_id_mensaje,
            paquete->header->payload_size);
}

void logger_mensaje_almacenado(t_mensaje_despachable* mensaje_despachable) {
    if(mensaje_despachable)
        log_info(logger, "\tMENSAJE ALMACENADO { id: %i | size: %i } ==> PARTICION ASIGNADA { base: %i | size: %i }",
                mensaje_despachable->id,
                mensaje_despachable->size,
                mensaje_despachable->particion_asociada->base,
                mensaje_despachable->particion_asociada->tamanio);
    else
        log_warning(logger_debug, "El MENSAJE fue ignorado por ser REDUNDANTE");
}

void logger_anda_a_saber_que_llego() {
    log_error(logger_debug, "Anda a saber que le llego al pobre BROKER :(");
}

void logger_dump_ejecutado(char* dump_file_path) {
    log_info(logger, "< DUMP DE CACHE EJECUTADO > { path_archivo: %s }", dump_file_path);
}

void logger_particiones_consolidadas(t_memoria* memoria, t_particion* una_particion, t_particion* otra_particion, int una_posicion, int otra_posicion) {
    if(memoria->algoritmo_memoria == BUDDY_SYSTEM)
        log_info(logger, "\tPARTICION %i { base: %i | size: %i } CONSOLIDADA CON PARTICION %i { base: %i | size: %i }",
                una_posicion, una_particion->base, una_particion->tamanio,
                otra_posicion, otra_particion->base, otra_particion->tamanio);
    else
        log_debug(logger_debug, "Particion %i CONSOLIDADA con Particion %i", una_posicion, otra_posicion);
}

void logger_mensaje_sin_despachar_eliminado(t_cola* cola, t_mensaje_despachable* mensaje_sin_despachar) {
    log_warning(logger_debug, "%s { id: %i } eliminado antes de ser DESPACHADO",
        mensaje_get_tipo_as_string(cola->tipo_mensaje),
        mensaje_sin_despachar->id);
}

void logger_archivo_no_encontrado(char* file_path) {
    log_error(logger_debug, "Archivo '%s' no encontrado\n", file_path);
}

void logger_archivo_invalido(char* file_path) {
    log_error(logger_debug, "Archivo '%s' posee configuraciones invalidas\n", file_path);
}

void logger_mensaje_enviado(t_paquete* paquete, t_suscriptor* suscriptor) {
    log_info(logger, "\t%s ENVIADO { id: %i | correlation_id: %i } A SUSCRIPTOR { id: %i }",
            mensaje_get_tipo_as_string(paquete->header->tipo_mensaje),
            paquete->header->id_mensaje,
            paquete->header->correlation_id_mensaje,
            suscriptor->id);
}

void logger_compactacion_ejecutada() {
    log_info(logger, "\tCOMPACTACION EJECUTADA");
}

void logger_mensaje_eliminado(uint32_t id_mensaje, t_particion* particion_asociada) {
    log_info(logger, "\tMENSAJE ELIMINADO { id: %i } ==> PARTICION LIBERADA { base: %i | size: %i }",
            id_mensaje,
            particion_asociada->base,
            particion_asociada->tamanio);
}

void logger_mensaje_tiene_todos_los_acks(t_mensaje_despachable* mensaje_despachable) {
    log_debug(logger_debug, "Llegaron todos los ACKs del MENSAJE { id: %i }", mensaje_despachable->id);
}

void logger_mensaje_eliminado_antes_de_recibir_ack(t_ack* ack) {
    log_warning(logger_debug, "No se pudo agregar el ACK del suscriptor %i. El MENSAJE { id: %i } ya fue eliminado",
            ack->id_suscriptor,
            ack->id_mensaje);
}

void logger_espacio_libre(t_memoria* memoria) {
    int _tamanio_particion_si_esta_libre_sino_0(t_particion* particion) {
        return particion->esta_libre ? particion->tamanio : 0;
    }

    log_debug(logger_debug, "Espacio libre: %ib", list_sum_by(memoria->particiones, (void*) _tamanio_particion_si_esta_libre_sino_0));
}

void logger_espacio_ocupado(t_memoria* memoria) {
    int _tamanio_particion_si_esta_ocupada_sino_0(t_particion* particion) {
        return !particion->esta_libre ? particion->tamanio : 0;
    }

    log_debug(logger_debug, "Espacio ocupado: %ib", list_sum_by(memoria->particiones, (void*) _tamanio_particion_si_esta_ocupada_sino_0));
}

void logger_detalle_memoria(t_memoria* memoria) {
    char* _detalle_particiones(t_list* particiones) {
        char* __concat_with_particion(char* acumulador, t_particion* particion) {
            char* detalle_particion = string_from_format("[%s=%i] ", particion->esta_libre ? "LI" : "OC", particion->tamanio);
            string_append(&acumulador, detalle_particion);
            free(detalle_particion);

            return acumulador;
        }

        return list_fold(particiones, (void*) string_new(), (void*) __concat_with_particion);
    }

    char* detalle = _detalle_particiones(memoria->particiones);
    log_debug(logger_debug, "Estado particiones: %s", detalle);
    free(detalle);
}
