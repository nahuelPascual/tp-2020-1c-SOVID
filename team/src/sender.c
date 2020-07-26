//
// Created by utnso on 9/05/20.
//

#include "sender.h"

extern t_log* logger;

static t_list* mensajes_esperando_respuesta;

static int esperar_id(int broker);

void sender_init_mensajes_esperando_respuesta() {
    mensajes_esperando_respuesta = list_create();
}

void enviar_get_pokemon(char* pokemon) {
	t_get_pokemon* mensaje = mensaje_crear_get_pokemon(pokemon);
	t_paquete* paquete = paquete_from_get_pokemon(mensaje);

	int broker = ipc_enviar_broker(paquete, config_team->ip_broker, config_team->puerto_broker);
	if (broker != -1) {
	    int id = esperar_id(broker);
        t_mensaje_enviado* pokemon_pedido = malloc(sizeof(t_mensaje_enviado));
        pokemon_pedido->id_mensaje = id;
        pokemon_pedido->mensaje_enviado = mensaje;
        list_add(mensajes_esperando_respuesta, pokemon_pedido);
	} else {
        mensaje_liberar_get_pokemon(mensaje);
	}

    ipc_cerrar(broker);
	paquete_liberar(paquete);
}

bool enviar_catch_pokemon(int id_entrenador, t_pokemon_mapeado* pokemon) {
    t_catch_pokemon* mensaje = mensaje_crear_catch_pokemon(pokemon->pokemon, pokemon->ubicacion->x, pokemon->ubicacion->y);
    t_paquete* paquete = paquete_from_catch_pokemon(mensaje);

    int broker = ipc_enviar_broker(paquete, config_team->ip_broker, config_team->puerto_broker);
    bool ok = broker != -1;
    if (ok) {
        int id = esperar_id(broker);
        t_mensaje_enviado* intento_captura = malloc(sizeof(t_mensaje_enviado));
        intento_captura->id_entrenador = id_entrenador;
        intento_captura->id_mensaje = id;
        intento_captura->mensaje_enviado = mensaje;
        list_add(mensajes_esperando_respuesta, intento_captura);
    } else {
        mensaje_liberar_catch_pokemon(mensaje);
    }

    ipc_cerrar(broker);
    paquete_liberar(paquete);

    return ok;
}

t_mensaje_enviado* get_mensaje_enviado(int id_mensaje) {
    bool _is_mensaje(void* msg) {
        t_mensaje_enviado* mensaje = (t_mensaje_enviado*) msg;
        return mensaje->id_mensaje == id_mensaje;
    }
    return list_remove_by_condition(mensajes_esperando_respuesta, (void*)_is_mensaje);
}

void liberar_get_pokemon_enviado(t_mensaje_enviado* c) {
    mensaje_liberar_get_pokemon(c->mensaje_enviado);
    free(c);
}

void liberar_catch_pokemon_enviado(t_mensaje_enviado* c) {
    mensaje_liberar_catch_pokemon(c->mensaje_enviado);
    free(c);
}

static int esperar_id(int broker) {
    t_paquete* paquete = ipc_recibir_de(broker);
    t_informe_id* informe_id = paquete_to_informe_id(paquete);

    logger_recibido(logger, paquete);

    int id = informe_id->id_mensaje;

    informe_id_liberar(informe_id);
    paquete_liberar(paquete);

    return id;
}
