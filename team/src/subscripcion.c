//
// Created by utnso on 9/05/20.
//

#include "subscripcion.h"

static void procesar_appeared_pokemon_(char* nombre, t_coord* posicion);
static void procesar_appeared_pokemon(t_appeared_pokemon* appeared_pokemon);
static void procesar_localized_pokemon(t_localized_pokemon* localized_pokemon);
static void procesar_caught_pokemon(t_paquete* paquete);
static void escuchar_en(int con);

void escuchar() {
    int server = ipc_escuchar_en("127.0.0.1", "8082"); //TODO levantar de config
    while(1) {
        int cliente = ipc_esperar_cliente(server);
        escuchar_en(cliente);
    }
}

void suscribirse_a(t_tipo_mensaje tipo_mensaje) {
	pthread_t thread;
	int servidor = enviar_suscripcion(tipo_mensaje);
	pthread_create(&thread, NULL, (void*)escuchar_en, servidor);
	pthread_detach(thread);
}

static void procesar_appeared_pokemon(t_appeared_pokemon* appeared_pokemon) {
    printf("Recibido APPEARED_POKEMON (%s)\n", appeared_pokemon->nombre);
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
    printf("Recibido LOCALIZED_POKEMON (%s)\n", localized_pokemon->nombre);
    if (is_pokemon_conocido(localized_pokemon->nombre)) {
        return;
    }
    for (int i=0 ; i<list_size(localized_pokemon->posiciones) ; i++) {
        t_coord* posicion = (t_coord*) list_get(localized_pokemon->posiciones, i);
        procesar_appeared_pokemon_(localized_pokemon->nombre, posicion);
    }
}

static void procesar_caught_pokemon(t_paquete* paquete) {
    printf("Recibido CAUGHT_POKEMON (%d)\n", paquete->header->correlation_id_mensaje);
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

static void escuchar_en(int con) {
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
            puts("Recibido mensaje invalido");
            continue;
        }

        enviar_ack(paquete->header->id_mensaje);
    }
}
