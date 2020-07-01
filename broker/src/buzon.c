/*
 * buzon.c
 *
 *  Created on: 18 jun. 2020
 *      Author: utnso
 */

#include "buzon.h"

t_buzon* buzon_crear() {
    t_buzon* buzon = malloc(sizeof(t_buzon));
    buzon->administrador_colas = administrador_colas_crear();
    buzon->memoria = memoria_crear(0, 1024, FIRST_FIT);

    return buzon;
}

bool buzon_almacenar_mensaje(t_buzon* buzon, t_paquete* paquete) {
    t_cola* cola = administrador_colas_get_cola_from(buzon->administrador_colas, paquete->header->tipo_mensaje);

    if(!cola_es_mensaje_redundante(cola, paquete)) {
        administrador_colas_asignar_id_mensaje_a(buzon->administrador_colas, paquete);

        t_mensaje_despachable* mensaje_despachable = mensaje_despachable_from_paquete(paquete, buzon->memoria);

        memoria_dividir_particion_si_es_mas_grande_que(buzon->memoria, mensaje_despachable->particion_asociada, mensaje_despachable->size);

        memoria_asignar_paquete_a_la_particion(buzon->memoria, paquete, mensaje_despachable->particion_asociada);

        cola_push_mensaje_sin_despachar(cola, mensaje_despachable);

        return true;
    }

    return false;
}

void buzon_despachar_mensaje_de(t_buzon* buzon, t_cola* cola) {
    t_mensaje_despachable* mensaje_despachable = cola_pop_mensaje_sin_despachar(cola);

    void _despachar_mensaje_a(t_suscriptor* suscriptor) {
        t_paquete* paquete = mensaje_despachable_to_paquete(mensaje_despachable, buzon->memoria);
        bool enviado = ipc_enviar_a(suscriptor->socket_asociado, paquete);
        paquete_liberar(paquete);

        if(enviado) {
            mensaje_despachable_add_suscriptor_enviado(mensaje_despachable, suscriptor);
        }
    }

    cola_iterate_suscriptores(cola, _despachar_mensaje_a);

    cola_push_mensaje_despachado(cola, mensaje_despachable);
}

void buzon_registrar_suscriptor(t_buzon* buzon, t_suscriptor* suscriptor) {
    t_cola* cola = administrador_colas_get_cola_from(buzon->administrador_colas, suscriptor->tipo_mensaje);

    cola_add_or_update_suscriptor(cola, suscriptor);

    void _enviar_al_suscriptor_si_no_lo_recibio(t_mensaje_despachable* mensaje_despachable) {
        bool recibido = mensaje_despachable_fue_recibido_por(mensaje_despachable, suscriptor);

        if(!recibido) {
            t_paquete* paquete = mensaje_despachable_to_paquete(mensaje_despachable, buzon->memoria);

            ipc_enviar_a(suscriptor->socket_asociado, paquete);

            paquete_liberar(paquete);
        }
    }

    cola_iterate_mensajes_despachados(cola, _enviar_al_suscriptor_si_no_lo_recibio);
}

void buzon_recibir_ack(t_buzon* buzon, t_ack* ack) {
    t_mensaje_despachable* mensaje_despachable = administrador_colas_find_mensaje_despachable_by_id(buzon->administrador_colas, ack->id_mensaje);

    mensaje_despachable_add_suscriptor_recibido(mensaje_despachable, ack);
}

void buzon_informar_id_mensaje_a(int id_mensaje, int socket_suscriptor) {
    t_informe_id* informe_id = informe_id_crear(id_mensaje);
    t_paquete* paquete = paquete_from_informe_id(informe_id);

    ipc_enviar_a(socket_suscriptor, paquete);

    informe_id_liberar(informe_id);
    paquete_liberar(paquete);
}
