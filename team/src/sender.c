//
// Created by utnso on 9/05/20.
//

#include "sender.h"

t_list* capturas_pendientes;
static int enviar_broker(t_paquete* p);

int enviar_suscripcion(t_tipo_mensaje tipo_mensaje) {
    t_suscripcion* s = suscripcion_crear(tipo_mensaje, 0);
    t_paquete* p = paquete_from_suscripcion(s);

    int broker = enviar_broker(p);

    suscripcion_liberar(s);
    paquete_liberar(p);

    return broker;
}

void enviar_get_pokemon(char* pokemon, void* cantidad) {
	t_get_pokemon* mensaje = mensaje_crear_get_pokemon(pokemon);
	t_paquete* paquete = paquete_from_get_pokemon(mensaje);

	enviar_broker(paquete);

	mensaje_liberar_get_pokemon(mensaje);
	paquete_liberar(paquete);
}

void enviar_catch_pokemon(t_pokemon* pokemon) {
    t_catch_pokemon* mensaje = mensaje_crear_catch_pokemon(pokemon->pokemon, pokemon->ubicacion->x, pokemon->ubicacion->y);
    t_paquete* paquete = paquete_from_catch_pokemon(mensaje);

    enviar_broker(paquete);

    mensaje_liberar_catch_pokemon(mensaje);
    paquete_liberar(paquete);
}

void enviar_ack(uint32_t id_mensaje) {
    t_ack* ack = ack_crear(id_mensaje);
    t_paquete* paquete = paquete_from_ack(ack);

    enviar_broker(paquete);

    ack_liberar(ack);
    paquete_liberar(paquete);
}

t_captura* get_mensaje_enviado(int id_mensaje) { //TODO orden superior
    for (int i=0 ; i>list_size(capturas_pendientes) ; i++) {
        t_captura* captura = list_get(capturas_pendientes, i);
        if (captura->id_mensaje == id_mensaje) {
            return (t_captura*) list_remove(capturas_pendientes, i);
        }
    }
    return NULL;
}

static int enviar_broker(t_paquete* p) {
    int broker = ipc_conectarse_a(config_team->ip_broker, config_team->puerto_broker);
    ipc_enviar_a(broker, p);
    ipc_cerrar(broker);
    return broker;
}
