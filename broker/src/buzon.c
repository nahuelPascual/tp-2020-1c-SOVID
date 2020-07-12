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
    buzon->memoria = memoria_crear(0, 1024, PARTICIONES_DINAMICAS, FIRST_FIT, FIFO, 3);

    return buzon;
}

void buzon_almacenar_mensaje(t_buzon* buzon, t_paquete* paquete) {
    t_cola* cola = administrador_colas_get_cola_from(buzon->administrador_colas, paquete->header->tipo_mensaje);

    if(!cola_es_mensaje_redundante(cola, paquete)) {
        buzon_vaciar_hasta_tener(buzon, paquete->header->payload_size);

        t_mensaje_despachable* mensaje_despachable = mensaje_despachable_from_paquete(paquete, buzon->memoria);

        administrador_colas_asignar_id_mensaje_a(buzon->administrador_colas, mensaje_despachable);

        memoria_dividir_particion_si_es_mas_grande_que(buzon->memoria, mensaje_despachable->particion_asociada, mensaje_despachable->size);

        memoria_asignar_paquete_a_la_particion(buzon->memoria, paquete, mensaje_despachable->particion_asociada);

        cola_push_mensaje_sin_despachar(cola, mensaje_despachable);
    }
}

void buzon_despachar_mensaje_de(t_buzon* buzon, t_cola* cola) {
    t_mensaje_despachable* mensaje_despachable = cola_pop_mensaje_sin_despachar(cola);

    void _despachar_mensaje_a(t_suscriptor* suscriptor) {
        t_paquete* paquete = mensaje_despachable_to_paquete(mensaje_despachable, buzon->memoria);
        paquete_set_tipo_mensaje(paquete, cola->tipo_mensaje);
        bool enviado = ipc_enviar_a(suscriptor->socket_asociado, paquete);
        paquete_liberar(paquete);

        if(enviado) {
            mensaje_despachable_add_suscriptor_enviado(mensaje_despachable, suscriptor);
        }
    }

    cola_iterate_suscriptores(cola, _despachar_mensaje_a);

    cola_push_mensaje_despachado(cola, mensaje_despachable);
}

void buzon_vaciar_hasta_tener(t_buzon* buzon, int espacio) {
    while(!memoria_existe_particion_libre_con(buzon->memoria, espacio)) {
        if(memoria_corresponde_compactar(buzon->memoria)) {
            memoria_compactar(buzon->memoria);

            memoria_resetear_contador_particiones_desocupadas(buzon->memoria);

            if(memoria_existe_particion_libre_con(buzon->memoria, espacio))
                break;
        }

        t_particion* particion_victima = memoria_get_particion_a_desocupar(buzon->memoria);

        administrador_colas_remove_and_destroy_mensaje_despachable_by_id(buzon->administrador_colas, particion_victima->id_mensaje_asociado);
        memoria_desocupar_particion(buzon->memoria, particion_victima);

        memoria_aumentar_contador_particiones_desocupadas(buzon->memoria);
    }
}

void buzon_registrar_suscriptor(t_buzon* buzon, t_suscriptor* suscriptor) {
    t_cola* cola = administrador_colas_get_cola_from(buzon->administrador_colas, suscriptor->tipo_mensaje);

    cola_add_or_update_suscriptor(cola, suscriptor);

    void _enviar_al_suscriptor_si_no_lo_recibio(t_mensaje_despachable* mensaje_despachable) {
        bool recibido = mensaje_despachable_fue_recibido_por(mensaje_despachable, suscriptor);

        if(!recibido) {
            t_paquete* paquete = mensaje_despachable_to_paquete(mensaje_despachable, buzon->memoria);
            paquete_set_tipo_mensaje(paquete, cola->tipo_mensaje);
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

void buzon_informar_id_mensaje_a(t_buzon* buzon, int socket_suscriptor) {
    t_informe_id* informe_id = informe_id_crear(buzon->administrador_colas->id_mensaje);
    t_paquete* paquete = paquete_from_informe_id(informe_id);

    ipc_enviar_a(socket_suscriptor, paquete);

    informe_id_liberar(informe_id);
    paquete_liberar(paquete);
}
