//
// Created by utnso on 9/05/20.
//

#include "subscripcion.h"

static void procesar_new_pokemon(t_paquete*);
static void procesar_get_pokemon(t_paquete*);
static void procesar_catch_pokemon(t_paquete*);
static void enviar_appeared_pokemon_response(t_new_pokemon*, uint32_t);
static void enviar_localized_pokemon_response(char*, t_localized_info*, uint32_t);
static void enviar_caught_pokemon_response(bool, uint32_t);

int escuchar_a(int con){
    while(ipc_hay_datos_para_recibir_de(con)){
        t_paquete* paquete = ipc_recibir_de(con);
        pthread_t thread;
        switch(paquete->header->tipo_mensaje){
        case GET_POKEMON:
            pthread_create(&thread, NULL, (void*)procesar_get_pokemon, paquete);
            pthread_detach(thread);
            break;
        case NEW_POKEMON:
            pthread_create(&thread, NULL, (void*)procesar_new_pokemon, paquete);
            pthread_detach(thread);
            break;
        case CATCH_POKEMON:
            pthread_create(&thread, NULL, (void*)procesar_catch_pokemon, paquete);
            pthread_detach(thread);
            break;
        default:
            log_error(logger, "Recibido mensaje invalido: tipo de mensaje %d", paquete->header->tipo_mensaje);
            continue;
        }
        ipc_enviar_ack(config_game_card->id ,paquete->header->id_mensaje, con);
    }
    return 1;
}

static void procesar_get_pokemon(t_paquete* paquete){
    logger_recibido(logger, paquete);

    t_get_pokemon* get_pokemon =  paquete_to_get_pokemon(paquete);
    uint32_t correlation_id = paquete->header->id_mensaje;

    t_localized_info* localized_info = fs_get_pokemon(get_pokemon);

    enviar_localized_pokemon_response(get_pokemon->nombre, localized_info, correlation_id);

    mensaje_liberar_get_pokemon(get_pokemon);
    free(localized_info);
    paquete_liberar(paquete);
}

static void enviar_localized_pokemon_response(char* nombre_pokemon, t_localized_info* localized_info, uint32_t correlation_id){
    t_localized_pokemon* localized_pokemon = mensaje_crear_localized_pokemon(nombre_pokemon, localized_info->cantidad_coordenadas, localized_info->coordenadas);
    t_paquete* paquete = paquete_from_localized_pokemon(localized_pokemon);
    paquete->header->correlation_id_mensaje = correlation_id;
    ipc_enviar_broker(paquete, config_game_card->ip_broker, config_game_card->puerto_broker);

    mensaje_liberar_localized_pokemon(localized_pokemon);
    paquete_liberar(paquete);
}

static void procesar_new_pokemon(t_paquete* paquete){
    logger_recibido(logger, paquete);

    t_new_pokemon* new_pokemon = paquete_to_new_pokemon(paquete);
    uint32_t correlation_id = paquete->header->id_mensaje;

    fs_new_pokemon(new_pokemon);

    enviar_appeared_pokemon_response(new_pokemon, correlation_id);

    mensaje_liberar_new_pokemon(new_pokemon);
    paquete_liberar(paquete);
}

static void enviar_appeared_pokemon_response(t_new_pokemon* new_pokemon, uint32_t correlation_id){
    t_appeared_pokemon* appeared_pokemon = mensaje_crear_appeared_pokemon(new_pokemon->nombre, new_pokemon->posicion->x, new_pokemon->posicion->y);
    t_paquete* paquete = paquete_from_appeared_pokemon(appeared_pokemon);
    paquete->header->correlation_id_mensaje = correlation_id;
    ipc_enviar_broker(paquete, config_game_card->ip_broker, config_game_card->puerto_broker);

    mensaje_liberar_appeared_pokemon(appeared_pokemon);
    paquete_liberar(paquete);
}

static void procesar_catch_pokemon(t_paquete* paquete){
    logger_recibido(logger, paquete);

    t_catch_pokemon* catch_pokemon = paquete_to_catch_pokemon(paquete);
    uint32_t correlation_id = paquete->header->id_mensaje;

    bool is_caught = fs_catch_pokemon(catch_pokemon);

    enviar_caught_pokemon_response(is_caught, correlation_id);

    mensaje_liberar_catch_pokemon(catch_pokemon);
    paquete_liberar(paquete);
}

static void enviar_caught_pokemon_response(bool is_caught, uint32_t correlation_id){
   t_caught_pokemon* caught_pokemon = mensaje_crear_caught_pokemon(is_caught);
   t_paquete* paquete = paquete_from_caught_pokemon(caught_pokemon);
   paquete->header->correlation_id_mensaje = correlation_id;
   ipc_enviar_broker(paquete, config_game_card->ip_broker, config_game_card->puerto_broker);

   mensaje_liberar_caught_pokemon(caught_pokemon);
   paquete_liberar(paquete);
}
