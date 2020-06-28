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
    header->tipo_mensaje = mensaje_despachable->tipo;
    header->id_mensaje = mensaje_despachable->id;
    header->correlation_id_mensaje = mensaje_despachable->correlation_id;
    header->payload_size = mensaje_despachable->size;

    return header;
}

t_mensaje_despachable* mensaje_despachable_from_paquete(t_paquete* paquete, t_memoria* memoria) {
  t_particion* particion = memoria_buscar_particion_libre_con(memoria, paquete->header->payload_size);

  t_mensaje_despachable* mensaje_despachable = malloc(sizeof(t_mensaje_despachable));

  mensaje_despachable->tipo = paquete->header->tipo_mensaje;
  mensaje_despachable->id = paquete->header->id_mensaje;
  mensaje_despachable->correlation_id = paquete->header->correlation_id_mensaje;
  mensaje_despachable->size = paquete->header->payload_size;

  mensaje_despachable->particion_asociada = particion;

  mensaje_despachable->suscriptores_a_los_que_fue_enviado = list_create();
  mensaje_despachable->suscriptores_que_lo_recibieron = list_create();

  pthread_mutex_init(&mensaje_despachable->mutex_ack, NULL);

  return mensaje_despachable;
}

t_paquete* mensaje_despachable_to_paquete(t_mensaje_despachable* mensaje_despachable, t_memoria* memoria) {
  t_paquete* paquete = malloc(sizeof(t_paquete));

  paquete->header = _rearmar_header_con(mensaje_despachable);
  paquete->payload = malloc(mensaje_despachable->size);

  void* direccion_fisica = memoria_get_direccion_fisica(memoria, mensaje_despachable->particion_asociada->base);

  memcpy(paquete->payload, direccion_fisica, mensaje_despachable->size);

  return paquete;
}

void mensaje_despachable_liberar(t_mensaje_despachable* mensaje_despachable) {
    memoria_liberar_particion(mensaje_despachable->particion_asociada);
    list_destroy(mensaje_despachable->suscriptores_a_los_que_fue_enviado);
    list_destroy(mensaje_despachable->suscriptores_que_lo_recibieron);
    pthread_mutex_destroy(&mensaje_despachable->mutex_ack);
    free(mensaje_despachable);
}

void mensaje_despachable_agregar_ack(t_mensaje_despachable* mensaje_despachable, uint32_t id_suscriptor) {
    list_add(mensaje_despachable->ids_suscriptores_que_lo_recibieron, (void*) id_suscriptor);
}

bool mensaje_despachable_tiene_todos_los_acks(t_mensaje_despachable* mensaje_despachable) {
    bool _igualigual(int id_un_suscriptor, int id_otro_suscriptor) {
        return id_un_suscriptor == id_otro_suscriptor;
    }

    return list_equals(mensaje_despachable->ids_suscriptores_a_los_que_fue_enviado,
                       mensaje_despachable->ids_suscriptores_que_lo_recibieron,
                       (void*) _igualigual);
}

t_mensaje_despachable* mensaje_despachable_find_by_id_in(t_list* lista, uint32_t id) {
    bool _is_the_one(t_mensaje_despachable* mensaje_despachable) {
        return mensaje_despachable->id == id;
    }

    return list_find(lista, (void*) _is_the_one);
}

void mensaje_despachable_remove_by_id_from(t_list* lista, uint32_t id) {
    bool _is_the_one(t_mensaje_despachable* mensaje_despachable) {
        return mensaje_despachable->id == id;
    }

    list_remove_by_condition(lista, (void*) _is_the_one);
}
