//
// Created by utnso on 9/05/20.
//

#include "subscripcion.h"

extern t_log* default_logger;
extern t_log* logger;

static void procesar_appeared_pokemon_(char*, t_coord*);
static void procesar_appeared_pokemon(t_paquete*);
static void procesar_localized_pokemon(t_paquete*);
static void procesar_caught_pokemon(t_paquete*);

static void procesar_appeared_pokemon(t_paquete* paquete) {
    t_appeared_pokemon* appeared_pokemon = paquete_to_appeared_pokemon(paquete);
    procesar_appeared_pokemon_(appeared_pokemon->nombre, appeared_pokemon->posicion);
    mensaje_liberar_appeared_pokemon(appeared_pokemon);
    paquete_liberar(paquete);
}

static void procesar_appeared_pokemon_(char* nombre, t_coord* posicion) {
    pthread_mutex_lock(&mx_atrapados);
    int capturas_pendientes = objetivos_cantidad_pendientes(nombre);
    if (capturas_pendientes<1) {
        pthread_mutex_unlock(&mx_atrapados);
        log_debug(default_logger, "Se ignora %s porque no es un pokemon requerido", nombre);
        return;
    }

    t_pokemon_mapeado* objetivo = pokemon_agregar_al_mapa(nombre, 1, posicion);

    pthread_mutex_lock(&mx_entrenadores);
    int entrenadores_asignados = entrenador_cantidad_asignado_a(nombre);
    if (entrenadores_asignados >= capturas_pendientes) {
        log_debug(default_logger, "No se planifica %s porque ya hay %d capturas en proceso", nombre, entrenadores_asignados);
        pthread_mutex_unlock(&mx_entrenadores);
        pthread_mutex_unlock(&mx_atrapados);
        return;
    }

    t_entrenador* entrenador = entrenador_get_libre_mas_cercano(posicion);
    if (entrenador == NULL) {
        log_debug(default_logger, "No hay ningun entrenador libre para planificar hacia (%d, %d)", posicion->x, posicion->y);
        pthread_mutex_unlock(&mx_entrenadores);
        pthread_mutex_unlock(&mx_atrapados);
        return;
    }

    entrenador->pokemon_buscado = objetivo;
    planificador_encolar_ready(entrenador);

    pthread_mutex_unlock(&mx_entrenadores);
    pthread_mutex_unlock(&mx_atrapados);
}

static void procesar_localized_pokemon(t_paquete* paquete) {
    t_localized_pokemon* localized_pokemon = paquete_to_localized_pokemon(paquete);

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
    paquete_liberar(paquete);
}

static void procesar_caught_pokemon(t_paquete* paquete) {
    t_mensaje_enviado* intento_captura = get_mensaje_enviado(paquete->header->correlation_id_mensaje);
    if (intento_captura == NULL) {
        log_debug(default_logger, "Se ignora el mensaje por no corresponder a este team");
        paquete_liberar(paquete);
        return;
    }

    t_caught_pokemon* caught_pokemon = paquete_to_caught_pokemon(paquete);
    t_entrenador* entrenador = entrenador_get(intento_captura->id_entrenador);
    t_catch_pokemon* mensaje = (t_catch_pokemon*) intento_captura->mensaje_enviado;

    if (caught_pokemon->is_caught){
        log_debug(default_logger, "El entrenador #%d capturo un %s", entrenador->id, mensaje->nombre);
        entrenador_concretar_captura(entrenador, mensaje->nombre, mensaje->posicion);
    } else {
        log_debug(default_logger, "El entrenador #%d no pudo capturar a %s", entrenador->id, mensaje->nombre);
        pokemon_sacar_del_mapa(mensaje->nombre, mensaje->posicion);
        entrenador->pokemon_buscado = NULL; // es el mismo puntero que el del mapa y ya se libera en la funcion de arriba
    }

    entrenador_verificar_objetivos(entrenador);
    if (entrenador->estado == BLOCKED_IDLE) {
        if (!caught_pokemon->is_caught) planificador_reasignar(mensaje->nombre); // se planifica a algun entrenador (el mas cercano) para capturar esta especie en particular
        planificador_admitir(entrenador); // se replanifica al entrenador, si hubiera mas objetivos asignbles en el mapa
    } else {
        planificador_verificar_deadlock_exit(entrenador);
    }

    liberar_catch_pokemon_enviado(intento_captura);
    mensaje_liberar_caught_pokemon(caught_pokemon);
    paquete_liberar(paquete);
}

int escuchar_a(int con) {
    while(ipc_hay_datos_para_recibir_de(con)){
        t_paquete* paquete = ipc_recibir_de(con);
        logger_recibido(logger, paquete);

        pthread_t thread;
        switch(paquete->header->tipo_mensaje) {
        case LOCALIZED_POKEMON:
            pthread_create(&thread, NULL, (void*)procesar_localized_pokemon, paquete);
            pthread_detach(thread);
            break;
        case APPEARED_POKEMON:
            pthread_create(&thread, NULL, (void*)procesar_appeared_pokemon, paquete);
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
        ipc_enviar_ack(config_team->id, paquete->header->id_mensaje, con);
    }
    return 1;
}
