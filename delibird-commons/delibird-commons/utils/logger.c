//
// Created by utnso on 16/05/20.
//
#include "logger.h"

static void loggerAck(t_log* logger, t_paquete* paquete);
static void loggerSuscripcion(t_log* logger, t_paquete* paquete);
static void loggerInforme(t_log* logger, t_paquete* paquete);

static char* crearPosicionesConcatenadas(t_localized_pokemon* localized_pokemon);
static char* crearInfoHeader(t_paquete* paquete);

static char* crearInfoNewPokemon(t_new_pokemon* new_pokemon);
static char* crearInfoLocalizedPokemon(t_localized_pokemon* localized_pokemon);
static char* crearInfoGetPokemon(t_get_pokemon* get_pokemon);
static char* crearInfoAppearedPokemon(t_appeared_pokemon* appeared_pokemon);
static char* crearInfoCatchPokemon(t_catch_pokemon* catch_pokemon);
static char* crearInfoCaughtPokemon(t_caught_pokemon* caught_pokemon);

static char* crearInfoAck(t_ack* ack);
static char* crearInfoSuscripcion(t_suscripcion* suscripcion);
static char* crearInfoInforme(t_informe_id* informe_id);

static char* crearStringMensaje(t_paquete* paquete);
static char* crearStringPaquete(t_paquete* paquete);

typedef enum {
    CORRECTO, ERROR_MENSAJE, ERROR_PAQUETE
} t_error_logger;

static t_error_logger flag_error;
static void resetFlagError(t_error_logger flag);

static char* crearPosicionesConcatenadas(t_localized_pokemon* localized_pokemon){
    int i;
    char* posiciones = string_new();
    for(i=0; i < localized_pokemon->posiciones_len; i++){
        t_coord* pos = (t_coord*)list_get(localized_pokemon->posiciones, i);
        string_append_with_format(&posiciones, "(%d,%d) ", pos->x, pos->y);
    }
    return posiciones;
}

static char* crearInfoHeader(t_paquete* paquete){
    char* info = string_new();
    string_append_with_format(&info, "Id_mensaje: %d | Correlation_id_mensaje: %d | Payload_size: %d",
        paquete->header->id_mensaje,
        paquete->header->correlation_id_mensaje,
        paquete->header->payload_size);
    return info;
}

static char* crearInfoNewPokemon(t_new_pokemon* new_pokemon){
    char* log_new_pokemon= string_new();
    string_append_with_format(&log_new_pokemon, "NEW POKEMON - Nombre_len: %d | Nombre: %s | Posicion: (%d,%d) | Cantidad: %d",
            new_pokemon->nombre_len,
            new_pokemon->nombre,
            new_pokemon->posicion->x, new_pokemon->posicion->y,
            new_pokemon->cantidad);
    return log_new_pokemon;
}

static char* crearInfoLocalizedPokemon(t_localized_pokemon* localized_pokemon){
    char* log_localized_pokemon= string_new();
    char* posiciones = crearPosicionesConcatenadas(localized_pokemon);
    string_append_with_format(&log_localized_pokemon, "LOCALIZED POKEMON - Nombre_len: %d | Nombre: %s | Posiciones_len: %d | Posiciones: %s",
            localized_pokemon->nombre_len,
            localized_pokemon->nombre,
            localized_pokemon->posiciones_len,
            posiciones);
    free(posiciones);
    return log_localized_pokemon;
}

static char* crearInfoGetPokemon(t_get_pokemon* get_pokemon){
    char* log_get_pokemon= string_new();
    string_append_with_format(&log_get_pokemon, "GET POKEMON - Nombre_len: %d | Nombre: %s",
            get_pokemon->nombre_len,
            get_pokemon->nombre);
    return log_get_pokemon;
}

static char* crearInfoAppearedPokemon(t_appeared_pokemon* appeared_pokemon){
    char* log_appeared_pokemon= string_new();
    string_append_with_format(&log_appeared_pokemon, "APPEARED POKEMON - Nombre_len: %d | Nombre: %s | Posicion: (%d,%d)",
            appeared_pokemon->nombre_len,
            appeared_pokemon->nombre,
            appeared_pokemon->posicion->x, appeared_pokemon->posicion->y);
    return log_appeared_pokemon;
}

static char* crearInfoCatchPokemon(t_catch_pokemon* catch_pokemon){
    char* log_catch_pokemon= string_new();
    string_append_with_format(&log_catch_pokemon, "CATCH POKEMON - Nombre_len: %d | Nombre: %s | Posicion: (%d,%d)",
            catch_pokemon->nombre_len,
            catch_pokemon->nombre,
            catch_pokemon->posicion->x, catch_pokemon->posicion->y);
    return log_catch_pokemon;
}

static char* crearInfoCaughtPokemon(t_caught_pokemon* caught_pokemon){
    char* log_caught_pokemon= string_new();
    string_append_with_format(&log_caught_pokemon, "CAUGHT POKEMON - Is_caught: %s",
            caught_pokemon->is_caught ? "true" : "false");
    return log_caught_pokemon;
}

static char* crearInfoAck(t_ack* ack){
    char* infoAck = string_new();
    string_append_with_format(&infoAck, "ACK - Id_mensaje ACK: %d",
        ack->id_mensaje);
    return infoAck;
}

static char* crearInfoSuscripcion(t_suscripcion* suscripcion){
    char* infoSuscripcion = string_new();
    string_append_with_format(&infoSuscripcion, "SUSCRIPCION - Tipo_mensaje: %d | Tiempo: %d",
            suscripcion->tipo_mensaje,
            suscripcion->tiempo);
    return infoSuscripcion;
}

static char* crearInfoInforme(t_informe_id* informe_id){
    char* infoInforme = string_new();
    string_append_with_format(&infoInforme, "INFORME - Id_mensaje INFORME: %i",
        informe_id->id_mensaje);
    return infoInforme;
}

void logger_mensaje(t_log* logger, t_paquete* paquete){
    char* infoHeader = crearInfoHeader(paquete);
    switch(paquete->header->tipo_mensaje) {
    case NEW_POKEMON:{
        t_new_pokemon* new_pokemon = paquete_to_new_pokemon(paquete);
        char* infoNewPokemon = crearInfoNewPokemon(new_pokemon);

        log_info(logger, "*NEW_POKEMON* %s | %s ",
                infoHeader,
                infoNewPokemon);

        free(infoNewPokemon);
        mensaje_liberar_new_pokemon(new_pokemon);
        break;
    }
    case LOCALIZED_POKEMON:{
        t_localized_pokemon* localized_pokemon = paquete_to_localized_pokemon(paquete);
        char* infoLocalizedPokemon = crearInfoLocalizedPokemon(localized_pokemon);

        log_info(logger, "*LOCALIZED_POKEMON* %s | %s",
                infoHeader,
                infoLocalizedPokemon);

        free(infoLocalizedPokemon);
        mensaje_liberar_localized_pokemon(localized_pokemon);
        break;
    }
    case GET_POKEMON:{
        t_get_pokemon* get_pokemon = paquete_to_get_pokemon(paquete);
        char* infoGetPokemon = crearInfoGetPokemon(get_pokemon);

        log_info(logger, "*GET_POKEMON* %s | %s ",
            infoHeader,
            infoGetPokemon);

        free(infoGetPokemon);
        mensaje_liberar_get_pokemon(get_pokemon);
        break;
    }
    case APPEARED_POKEMON:{
        t_appeared_pokemon* appeared_pokemon = paquete_to_appeared_pokemon(paquete);
        char* infoAppearedPokemon = crearInfoAppearedPokemon(appeared_pokemon);

        log_info(logger, "*APPEARED_POKEMON* %s | %s",
            infoHeader,
            infoAppearedPokemon);

        free(infoAppearedPokemon);
        mensaje_liberar_appeared_pokemon(appeared_pokemon);
        break;
    }
    case CATCH_POKEMON:{
        t_catch_pokemon* catch_pokemon = paquete_to_catch_pokemon(paquete);
        char* infoCatchPokemon = crearInfoCatchPokemon(catch_pokemon);

        log_info(logger, "*CATCH_POKEMON* %s | %s ",
            infoHeader,
            infoCatchPokemon);

        free(infoCatchPokemon);
        mensaje_liberar_catch_pokemon(catch_pokemon);
        break;
    }
    case CAUGHT_POKEMON:{
        t_caught_pokemon* caught_pokemon = paquete_to_caught_pokemon(paquete);
        char* infoCaughtPokemon = crearInfoCaughtPokemon(caught_pokemon);

        log_info(logger, "*CAUGHT_POKEMON* %s | %s",
            infoHeader,
            infoCaughtPokemon);

        free(infoCaughtPokemon);
        mensaje_liberar_caught_pokemon(caught_pokemon);
        break;
    }
    default:
        log_error(logger, "Error al intentar loguear el mensaje. Tipo mensaje %d", paquete->header->tipo_mensaje);
        break;
    }
    free(infoHeader);
}

static void loggerAck(t_log* logger, t_paquete* paquete) {
    t_ack* ack = paquete_to_ack(paquete);
    char* infoAck = crearInfoAck(ack);
    char* infoPaquete = crearInfoHeader(paquete);

    log_info(logger, "*ACK* %s | %s",
        infoPaquete,
        infoAck);

    free(infoAck);
    free(infoPaquete);
    ack_liberar(ack);
}

static void loggerSuscripcion(t_log* logger, t_paquete* paquete) {
    t_suscripcion* suscripcion = paquete_to_suscripcion(paquete);
    char* infoSuscripcion = crearInfoSuscripcion(suscripcion);
    char* infoPaquete = crearInfoHeader(paquete);

    log_info(logger, "*SUSCRIPCION:* %s | %s ",
        infoPaquete,
        infoSuscripcion);

    free(infoSuscripcion);
    free(infoPaquete);
    suscripcion_liberar(suscripcion);
}

static void loggerInforme(t_log* logger, t_paquete* paquete) {
    t_informe_id* informe_id = paquete_to_informe_id(paquete);
    char* infoInforme = crearInfoInforme(informe_id);
    char* infoPaquete = crearInfoHeader(paquete);

    log_info(logger, "*ID* %s | %s ",
        infoPaquete,
        infoInforme);

    free(infoInforme);
    free(infoPaquete);
    informe_id_liberar(informe_id);
}

static void resetFlagError(t_error_logger flag){
    flag = CORRECTO;
}

void logger_conexion(t_log* logger, char* proceso,char* ip,char* puerto){
    log_info(logger,"*CONEXION %s* IP: %s | PUERTO: %s",
        proceso,
        ip,
        puerto);
}

void logger_new_pokemon(t_log* logger, t_new_pokemon* new_pokemon){
    char* infoNewPokemon = crearInfoNewPokemon(new_pokemon);
    log_info(logger, infoNewPokemon);
    free(infoNewPokemon);
}

void logger_localized_pokemon(t_log* logger, t_localized_pokemon* localized_pokemon){
    char* infoLocalizedPokemon = crearInfoLocalizedPokemon(localized_pokemon);
    log_info(logger, infoLocalizedPokemon);
    free(infoLocalizedPokemon);
}

void logger_get_pokemon(t_log* logger, t_get_pokemon* get_pokemon){
    char* infoGetPokemon = crearInfoGetPokemon(get_pokemon);
    log_info(logger, infoGetPokemon);
    free(infoGetPokemon);
}

void logger_appeared_pokemon(t_log* logger, t_appeared_pokemon* appeared_pokemon){
    char* infoAppearedPokemon = crearInfoAppearedPokemon(appeared_pokemon);
    log_info(logger, infoAppearedPokemon);
    free(infoAppearedPokemon);
}

void logger_catch_pokemon(t_log* logger, t_catch_pokemon* catch_pokemon){
    char* infoCatchPokemon = crearInfoCatchPokemon(catch_pokemon);
    log_info(logger, infoCatchPokemon);
    free(infoCatchPokemon);
}

void logger_caught_pokemon(t_log* logger, t_caught_pokemon* caught_pokemon){
    char* infoCaughtPokemon = crearInfoCaughtPokemon(caught_pokemon);
    log_info(logger, infoCaughtPokemon);
    free(infoCaughtPokemon);
}

void logger_ack(t_log* logger, t_ack* ack){
    char* infoAck = crearInfoAck(ack);
    log_info(logger, infoAck);
    free(infoAck);
}

void logger_suscripcion(t_log* logger, t_suscripcion* suscripcion){
    char* infoSuscripcion = crearInfoSuscripcion(suscripcion);
    log_info(logger, infoSuscripcion);
    free(infoSuscripcion);
}
void logger_informe(t_log* logger, t_informe_id* informe_id){
    char* infoInforme = crearInfoInforme(informe_id);
    log_info(logger, infoInforme);
    free(infoInforme);
}

void logger_paquete(t_paquete* paquete, t_log* logger){
    switch(paquete->header->tipo_paquete) {
            case MENSAJE:
                logger_mensaje(logger, paquete);
                break;
            case ACK:
                loggerAck(logger, paquete);
                break;
            case SUSCRIPCION:
                loggerSuscripcion(logger, paquete);
                break;
            case INFORME_ID:
                loggerInforme(logger, paquete);
                break;
            default:
                log_error(logger, "Error al intentar loguear el paquete. Tipo paquete %d", paquete->header->tipo_paquete);
                break;
            }
}

static char* crearStringPaquete(t_paquete* paquete){
    switch(paquete->header->tipo_paquete) {
        case MENSAJE:{
            return crearStringMensaje(paquete);
        }
        case ACK:{
            t_ack* ack = paquete_to_ack(paquete);
            char* mensajeAck = crearInfoAck(ack);
            ack_liberar(ack);
            return mensajeAck;
        }
        case SUSCRIPCION:{
            t_suscripcion* suscripcion = paquete_to_suscripcion(paquete);
            char* mensajeSuscripcion = crearInfoSuscripcion(suscripcion);
            suscripcion_liberar(suscripcion);
            return mensajeSuscripcion;
        }
        case INFORME_ID:{
            t_informe_id* informe = paquete_to_informe_id(paquete);
            char* mensajeInforme = crearInfoInforme(informe);
            informe_id_liberar(informe);
            return mensajeInforme;
        }
        default:{
            flag_error = ERROR_PAQUETE;
            return "ERROR PAQUETE";
        }
    }
}

static char* crearStringMensaje(t_paquete* paquete){
    char* respuesta;
    switch(paquete->header->tipo_mensaje) {
        case NEW_POKEMON:{
            t_new_pokemon* new_pokemon = paquete_to_new_pokemon(paquete);
            respuesta = crearInfoNewPokemon(new_pokemon);
            mensaje_liberar_new_pokemon(new_pokemon);
            return respuesta;
        }
        case LOCALIZED_POKEMON:{
            t_localized_pokemon* localized_pokemon = paquete_to_localized_pokemon(paquete);
            respuesta = crearInfoLocalizedPokemon(localized_pokemon);
            mensaje_liberar_localized_pokemon(localized_pokemon);
            return respuesta;
        }
        case GET_POKEMON:{
            t_get_pokemon* get_pokemon = paquete_to_get_pokemon(paquete);
            respuesta = crearInfoGetPokemon(get_pokemon);
            mensaje_liberar_get_pokemon(get_pokemon);
            return respuesta;
        }
        case APPEARED_POKEMON:{
            t_appeared_pokemon* appeared_pokemon = paquete_to_appeared_pokemon(paquete);
            respuesta = crearInfoAppearedPokemon(appeared_pokemon);
            mensaje_liberar_appeared_pokemon(appeared_pokemon);
            return respuesta;
        }
        case CATCH_POKEMON:{
            t_catch_pokemon* catch_pokemon = paquete_to_catch_pokemon(paquete);
            respuesta = crearInfoCatchPokemon(catch_pokemon);
            mensaje_liberar_catch_pokemon(catch_pokemon);
            return respuesta;
        }
        case CAUGHT_POKEMON:{
            t_caught_pokemon* caught_pokemon = paquete_to_caught_pokemon(paquete);
            respuesta = crearInfoCaughtPokemon(caught_pokemon);
            mensaje_liberar_caught_pokemon(caught_pokemon);
            return respuesta;
        }
        default:
            flag_error = ERROR_MENSAJE;
            return "ERROR MENSAJE";
    }
}

void logger_recibido(t_log* logger, t_paquete* paquete){
    resetFlagError(flag_error);
    char* info = crearStringPaquete(paquete);
    switch(flag_error){
        case CORRECTO:
            log_info(logger, "PAQUETE RECIBIDO - %s", info);
            free(info);
             return;

        case ERROR_MENSAJE:
            log_error(logger, "Error al intentar loguear el mensaje. Tipo mensaje %d", paquete->header->tipo_mensaje);
            return;

        case ERROR_PAQUETE:
            log_error(logger, "Error al intentar loguear el paquete. Tipo paquete %d", paquete->header->tipo_paquete);
            return;

        default:
            log_error(logger, "Error desconocido. Flag:%d", flag_error);
            return;
    }
}

void logger_enviado(t_log* logger, t_paquete* paquete){
    resetFlagError(flag_error);
    char* info = crearStringPaquete(paquete);
    switch(flag_error){
        case CORRECTO:
            log_info(logger, "PAQUETE ENVIADO - %s", info);
            free(info);
             return;

        case ERROR_MENSAJE:
            log_error(logger, "Error al intentar loguear el mensaje. Tipo mensaje %d", paquete->header->tipo_mensaje);
            return;

        case ERROR_PAQUETE:
            log_error(logger, "Error al intentar loguear el paquete. Tipo paquete %d", paquete->header->tipo_paquete);
            return;

        default:
            log_error(logger, "Error desconocido. Flag:%d", flag_error);
            return;
    }
}

