//
// Created by utnso on 9/05/20.
//

#include "subscripcion.h"

extern t_log* default_logger;

static void procesar_appeared_pokemon_(char*, t_coord*);
static void procesar_appeared_pokemon(t_appeared_pokemon*);
static void procesar_localized_pokemon(t_paquete*);
static void procesar_caught_pokemon(t_paquete*);
static void escuchar_a(int cliente);

void escuchar_gameboy() {
    int server = ipc_escuchar_en(config_team->ip_team, config_team->puerto_team);
    while(1) {
        int cliente = ipc_esperar_cliente(server);
        escuchar_a(cliente);
    }
}

void suscribirse_a(t_tipo_mensaje tipo_mensaje) {
	pthread_t thread;
	int broker = enviar_suscripcion(tipo_mensaje);
	pthread_create(&thread, NULL, (void*)escuchar_a, broker);
	pthread_detach(thread);
}

static void procesar_appeared_pokemon(t_appeared_pokemon* appeared_pokemon) {
    log_debug(default_logger, "Recibido APPEARED_POKEMON (%s)", appeared_pokemon->nombre);
    procesar_appeared_pokemon_(appeared_pokemon->nombre, appeared_pokemon->posicion);
    mensaje_liberar_appeared_pokemon(appeared_pokemon);
}

static void procesar_appeared_pokemon_(char* nombre, t_coord* posicion) {
    if (!is_pokemon_requerido(nombre)){
        log_debug(default_logger, "Se ignora %s porque no es un pokemon requerido", nombre);
        return;
    }

    t_pokemon_mapeado* objetivo = pokemon_agregar_al_mapa(nombre, 1, posicion);

    if (entrenador_asignado_a(objetivo)) {
        log_debug(default_logger, "No se planifica %s porque ya hay una captura en proceso para esa especie", nombre);
        return;
    }

    t_entrenador* entrenador = entrenador_get_libre_mas_cercano(posicion);
    if (entrenador == NULL) {
        log_debug(default_logger, "No hay ningun entrenador libre para planificar hacia (%d, %d)", posicion->x, posicion->y);
        return;
    }

    entrenador->pokemon_buscado = objetivo;
    planificador_encolar_ready(entrenador);
}

static void procesar_localized_pokemon(t_paquete* paquete) {
    t_localized_pokemon* localized_pokemon = paquete_to_localized_pokemon(paquete);
    log_debug(default_logger, "Recibido LOCALIZED_POKEMON (%s)", localized_pokemon->nombre);

    t_mensaje_enviado* pokemon_pedido = get_mensaje_enviado(paquete->header->correlation_id_mensaje);
    if (pokemon_pedido == NULL) {
        log_debug(default_logger, "Se ignora el mensaje por no corresponder a este team");
        mensaje_liberar_localized_pokemon(localized_pokemon);
        return;
    }

    if (is_pokemon_conocido(localized_pokemon->nombre)) {
        log_debug(default_logger, "Se ignora el mensaje porque ya se recibio un localized_pokemon o appeared_pokemon para %s", localized_pokemon->nombre);
        mensaje_liberar_localized_pokemon(localized_pokemon);
        liberar_get_pokemon_enviado(pokemon_pedido);
        return;
    }

    for (int i=0 ; i<list_size(localized_pokemon->posiciones) ; i++) {
        t_coord* posicion = (t_coord*) list_get(localized_pokemon->posiciones, i);
        procesar_appeared_pokemon_(localized_pokemon->nombre, posicion);
    }

    mensaje_liberar_localized_pokemon(localized_pokemon);
    liberar_get_pokemon_enviado(pokemon_pedido);
}

static void procesar_caught_pokemon(t_paquete* paquete) {
    log_debug(default_logger, "Recibido CAUGHT_POKEMON (id_catch_pokemon: %d)", paquete->header->correlation_id_mensaje);
    t_mensaje_enviado* intento_captura = get_mensaje_enviado(paquete->header->correlation_id_mensaje);
    if (intento_captura == NULL) {
        log_debug(default_logger, "Se ignora el mensaje por no corresponder a este team");
        return;
    }

    t_caught_pokemon* caught_pokemon = paquete_to_caught_pokemon(paquete);
    t_entrenador* entrenador = entrenador_get(intento_captura->id_entrenador);
    t_catch_pokemon* mensaje = intento_captura->mensaje_enviado;

    if (caught_pokemon->is_caught){
        entrenador_concretar_captura(entrenador, mensaje->nombre, mensaje->posicion);
        log_debug(default_logger, "El entrenador #%d capturo un %s", mensaje->nombre);
    } else {
        pokemon_sacar_del_mapa(mensaje->nombre, mensaje->posicion);
        entrenador->pokemon_buscado = NULL; // es el mismo puntero que el del mapa y ya se libera en la funcion de arriba
        log_debug(default_logger, "El entrenador #%d no pudo capturar a %s", mensaje->nombre);
    }

    entrenador_verificar_objetivos(entrenador);
    if (entrenador->estado == BLOCKED_IDLE) {
        planificador_admitir(entrenador);
    } else {
        planificador_verificar_deadlock_exit(entrenador);
    }

    liberar_catch_pokemon_enviado(intento_captura);
    mensaje_liberar_caught_pokemon(caught_pokemon);
}

static void escuchar_a(int cliente) {
    while(ipc_hay_datos_para_recibir_de(cliente)){
        t_paquete* paquete = ipc_recibir_de(cliente);

        pthread_t thread;
        switch(paquete->header->tipo_mensaje) {
        case LOCALIZED_POKEMON:
            pthread_create(&thread, NULL, (void*)procesar_localized_pokemon, paquete);
            pthread_detach(thread);
            break;
        case APPEARED_POKEMON:
            pthread_create(&thread, NULL, (void*)procesar_appeared_pokemon, paquete_to_appeared_pokemon(paquete));
            pthread_detach(thread);
            break;
        case CAUGHT_POKEMON:
            pthread_create(&thread, NULL, (void*)procesar_caught_pokemon, paquete);
            pthread_detach(thread);
            break;
        default:
            log_error(default_logger, "Recibido mensaje invalido: tipo de mensaje %d", paquete->header->tipo_mensaje);
            continue;
        }

        enviar_ack(paquete->header->id_mensaje, cliente);
        paquete_liberar(paquete);
    }
}
