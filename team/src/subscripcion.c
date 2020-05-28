//
// Created by utnso on 9/05/20.
//

#include "subscripcion.h"

static void procesar_appeared_pokemon_(char* nombre, t_coord* posicion);
static void procesar_appeared_pokemon(t_appeared_pokemon* appeared_pokemon);
static void procesar_localized_pokemon(t_localized_pokemon* localized_pokemon);
static void procesar_caught_pokemon(t_paquete* paquete);
static void escuchar_a(int con);

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
}

static void procesar_appeared_pokemon_(char* nombre, t_coord* posicion) {
    string_to_upper(nombre);
    if (!is_pokemon_requerido(nombre)){
        return;
    }
    pokemon_agregar_al_mapa(nombre, 1, posicion);

    t_entrenador* entrenador = entrenador_get_libre_mas_cercano(posicion);
    if (entrenador == NULL) {
        return;
    }
    entrenador_set_ready(entrenador);
    planificador_despertar();
}

static void procesar_localized_pokemon(t_localized_pokemon* localized_pokemon) {
    log_debug(default_logger, "Recibido LOCALIZED_POKEMON (%s)", localized_pokemon->nombre);
    if (is_pokemon_conocido(localized_pokemon->nombre)) {
        return;
    }
    for (int i=0 ; i<list_size(localized_pokemon->posiciones) ; i++) {
        t_coord* posicion = (t_coord*) list_get(localized_pokemon->posiciones, i);
        procesar_appeared_pokemon_(localized_pokemon->nombre, posicion);
    }
}

static void procesar_caught_pokemon(t_paquete* paquete) {
    log_debug(default_logger, "Recibido CAUGHT_POKEMON (id_catch_pokemon: %d)", paquete->header->correlation_id_mensaje);
    t_captura* intento_captura = get_mensaje_enviado(paquete->header->correlation_id_mensaje);
    if (intento_captura == NULL) {
        return;
    }

    t_caught_pokemon* caught_pokemon = paquete_to_caught_pokemon(paquete);
    t_entrenador* entrenador = entrenador_get(intento_captura->id_entrenador);
    if (caught_pokemon->is_caught){
        t_catch_pokemon* mensaje = intento_captura->mensaje_enviado;
        string_to_upper(mensaje->nombre);
        list_add(entrenador->capturados, mensaje->nombre);
        pokemon_sacar_del_mapa(mensaje->nombre, mensaje->posicion);
    }

    entrenador->estado = BLOCKED_IDLE;
}

static void escuchar_a(int con) {
    while(ipc_hay_datos_para_recibir_de(con)){
        t_paquete* paquete = ipc_recibir_de(con);

        pthread_t thread;
        switch(paquete->header->tipo_mensaje) {
        case LOCALIZED_POKEMON:
            pthread_create(&thread, NULL, (void*)procesar_localized_pokemon, paquete_to_localized_pokemon(paquete));
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

        enviar_ack(paquete->header->id_mensaje);
    }
}
