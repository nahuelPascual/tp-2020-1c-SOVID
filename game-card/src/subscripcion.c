//
// Created by utnso on 9/05/20.
//

#include "subscripcion.h"

static void procesar_new_pokemon(t_new_pokemon*);
static void procesar_get_pokemon(t_get_pokemon*);
static void procesar_catch_pokemon(t_catch_pokemon*);

void escuchar_a(int con) {
    while(ipc_hay_datos_para_recibir_de(con)){
        t_paquete* paquete = ipc_recibir_de(con);

        pthread_t thread;
        switch(paquete->header->tipo_mensaje) {
        case GET_POKEMON:
            pthread_create(&thread, NULL, (void*)procesar_get_pokemon, paquete_to_get_pokemon(paquete));
            pthread_detach(thread);
            break;
        case NEW_POKEMON:
            pthread_create(&thread, NULL, (void*)procesar_new_pokemon, paquete_to_new_pokemon(paquete));
            pthread_detach(thread);
            break;
        case CATCH_POKEMON:
            pthread_create(&thread, NULL, (void*)procesar_catch_pokemon, paquete_to_catch_pokemon(paquete));
            pthread_detach(thread);
            break;
        default:
            log_error(logger, "Recibido mensaje invalido: tipo de mensaje %d", paquete->header->tipo_mensaje);
            continue;
        }
        ipc_enviar_ack(config_game_card->id ,paquete->header->id_mensaje, con);
        paquete_liberar(paquete);
    }
}

static void procesar_get_pokemon(t_get_pokemon* get_pokemon) {
    log_info(logger, "Recibido GET POKEMON:");
    logger_get_pokemon(logger, get_pokemon);
}

static void procesar_new_pokemon(t_new_pokemon* new_pokemon) {
    log_info(logger, "Recibido NEW POKEMON:");
    logger_new_pokemon(logger, new_pokemon);
    t_pokemon_info* pokemon_info = check_pokemon_info(new_pokemon->nombre);
    if(pokemon_info->is_open){
        log_error(logger, "El archivo %s se encuentra abierto.", pokemon_info->file_path);
    }else{
        log_info(logger, "El archivo %s no se encuentra abierto", pokemon_info->file_path);
    }
}

static void procesar_catch_pokemon(t_catch_pokemon* catch_pokemon) {
    log_info(logger, "Recibido CATCH POKEMON:");
    logger_catch_pokemon(logger, catch_pokemon);
}
