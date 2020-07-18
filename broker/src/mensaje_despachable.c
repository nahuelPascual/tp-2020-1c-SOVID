/*
 * mensaje_despachable.c
 *
 *  Created on: 1 jun. 2020
 *      Author: utnso
 */

#include "mensaje_despachable.h"

t_header* _rearmar_header_con(t_mensaje_despachable* mensaje_despachable) {
    t_header* header = malloc(sizeof(t_header));

    header->tipo_paquete = MENSAJE;
    header->id_mensaje = mensaje_despachable->id;
    header->correlation_id_mensaje = mensaje_despachable->correlation_id;
    header->payload_size = mensaje_despachable->size;

    return header;
}

t_mensaje_despachable* mensaje_despachable_from_paquete(t_paquete* paquete, t_memoria* memoria) {
  t_particion* particion = memoria_buscar_particion_libre_con(memoria, paquete->header->payload_size);

  t_mensaje_despachable* mensaje_despachable = malloc(sizeof(t_mensaje_despachable));

  mensaje_despachable->correlation_id = paquete->header->correlation_id_mensaje;
  mensaje_despachable->size = paquete->header->payload_size;

  mensaje_despachable->particion_asociada = particion;

  mensaje_despachable->ids_suscriptores_a_los_que_fue_enviado = list_create();
  mensaje_despachable->ids_suscriptores_que_lo_recibieron = list_create();

  pthread_mutex_init(&mensaje_despachable->mutex_ids_suscriptores_a_los_que_fue_enviado, NULL);
  pthread_mutex_init(&mensaje_despachable->mutex_ids_suscriptores_que_lo_recibieron, NULL);

  return mensaje_despachable;
}

t_paquete* mensaje_despachable_to_paquete(t_mensaje_despachable* mensaje_despachable, t_memoria* memoria) {
  t_paquete* paquete = malloc(sizeof(t_paquete));

  paquete->header = _rearmar_header_con(mensaje_despachable);
  paquete->payload = malloc(mensaje_despachable->size);

  void* direccion_fisica = memoria_get_direccion_fisica(memoria, mensaje_despachable->particion_asociada->base);
  mensaje_despachable->particion_asociada->tiempo_ultima_referencia = clock();

  memcpy(paquete->payload, direccion_fisica, mensaje_despachable->size);

  return paquete;
}

void mensaje_despachable_liberar(t_mensaje_despachable* mensaje_despachable) {
    list_destroy(mensaje_despachable->ids_suscriptores_a_los_que_fue_enviado);
    list_destroy(mensaje_despachable->ids_suscriptores_que_lo_recibieron);
    pthread_mutex_destroy(&mensaje_despachable->mutex_ids_suscriptores_a_los_que_fue_enviado);
    pthread_mutex_destroy(&mensaje_despachable->mutex_ids_suscriptores_que_lo_recibieron);
    free(mensaje_despachable);
}

void mensaje_despachable_add_suscriptor_enviado(t_mensaje_despachable* mensaje_despachable, t_suscriptor* suscriptor) {
    pthread_mutex_lock(&mensaje_despachable->mutex_ids_suscriptores_a_los_que_fue_enviado);
    list_add(mensaje_despachable->ids_suscriptores_a_los_que_fue_enviado, (void*) suscriptor->id);
    pthread_mutex_unlock(&mensaje_despachable->mutex_ids_suscriptores_a_los_que_fue_enviado);
}

void mensaje_despachable_add_suscriptor_recibido(t_mensaje_despachable* mensaje_despachable, t_ack* ack) {
    pthread_mutex_lock(&mensaje_despachable->mutex_ids_suscriptores_que_lo_recibieron);
    list_add(mensaje_despachable->ids_suscriptores_que_lo_recibieron, (void*) ack->id_suscriptor);
    pthread_mutex_unlock(&mensaje_despachable->mutex_ids_suscriptores_que_lo_recibieron);
}

bool mensaje_despachable_tiene_todos_los_acks(t_mensaje_despachable* mensaje_despachable) {
    bool _igualigual(uint32_t id_un_suscriptor, uint32_t id_otro_suscriptor) {
        return id_un_suscriptor == id_otro_suscriptor;
    }

    return list_equals(mensaje_despachable->ids_suscriptores_a_los_que_fue_enviado,
                       mensaje_despachable->ids_suscriptores_que_lo_recibieron,
                       (void*) _igualigual);
}

bool mensaje_despachable_fue_enviado_a(t_mensaje_despachable* mensaje_despachable, t_suscriptor* suscriptor) {
    bool _is_the_one(uint32_t id_suscriptor) {
        return id_suscriptor == suscriptor->id;
    }

    pthread_mutex_lock(&mensaje_despachable->mutex_ids_suscriptores_a_los_que_fue_enviado);
    bool enviado = list_any_satisfy(mensaje_despachable->ids_suscriptores_a_los_que_fue_enviado, (void*) _is_the_one);
    pthread_mutex_unlock(&mensaje_despachable->mutex_ids_suscriptores_a_los_que_fue_enviado);

    return enviado;
}

bool mensaje_despachable_fue_recibido_por(t_mensaje_despachable* mensaje_despachable, t_suscriptor* suscriptor) {
    bool _is_the_one(uint32_t id_suscriptor) {
        return id_suscriptor == suscriptor->id;
    }

    pthread_mutex_lock(&mensaje_despachable->mutex_ids_suscriptores_que_lo_recibieron);
    bool recibido = list_any_satisfy(mensaje_despachable->ids_suscriptores_que_lo_recibieron, (void*) _is_the_one);
    pthread_mutex_unlock(&mensaje_despachable->mutex_ids_suscriptores_que_lo_recibieron);

    return recibido;
}

void mensaje_despachable_informar_id_a(t_mensaje_despachable* mensaje_despachable, int socket_suscriptor) {
    uint32_t id_a_informar = mensaje_despachable ? mensaje_despachable-> id : 0;

    t_informe_id* informe_id = informe_id_crear(id_a_informar);
    t_paquete* paquete = paquete_from_informe_id(informe_id);

    ipc_enviar_a(socket_suscriptor, paquete);

    informe_id_liberar(informe_id);
    paquete_liberar(paquete);
}

bool mensaje_despachable_es_misma_respuesta_que(t_mensaje_despachable* mensaje_despachable, t_paquete* paquete) {
    return mensaje_despachable->correlation_id == paquete->header->correlation_id_mensaje;
}
