//
// Created by utnso on 10/04/20.
//

#include "ipc.h"

static void ipc_escuchar_gameboy(t_listener_config*);
static void logs_broker_reintento();
static void logs_broker_error();

extern t_log* logger;

int ipc_escuchar_en(char* ip, char* puerto) {
    int socket_servidor;

    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip, puerto, &hints, &servinfo);

    for(p = servinfo; p != NULL; p = p->ai_next) {

        if((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            log_warning(logger, "Error en la creacion del socket");
            continue;
        }

        int i = 1;
        setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEPORT, &i, sizeof(i));
        if(bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_servidor);
            log_warning(logger, "Error al intentar asignar la direccion al socket");
            continue;
        }

        break;
    }

    listen(socket_servidor, SOMAXCONN);

    freeaddrinfo(servinfo);

    return socket_servidor;
}

int ipc_esperar_cliente(int socket_servidor) {
    struct sockaddr_in dir_cliente;

    socklen_t tam_direccion = sizeof(struct sockaddr_in);
    int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);

    return socket_cliente;
}

bool ipc_hay_datos_para_recibir_de(int socket) {
    int buffer;
    return recv(socket, &buffer, sizeof(int), MSG_PEEK) > 0;
}

t_paquete* ipc_recibir_de(int socket) {
    int received_bytes = 0;
    t_paquete* paquete = malloc(sizeof(t_paquete));
    t_header* header = malloc(sizeof(t_header));

    received_bytes += recv(socket, header, sizeof(t_header), MSG_WAITALL);
    paquete->payload = malloc(header->payload_size);

    received_bytes += recv(socket, paquete->payload, header->payload_size, MSG_WAITALL);
    paquete->header = header;

    int bytes = sizeof(t_header) + header->payload_size;

    return received_bytes == bytes ? paquete : (paquete_liberar(paquete), NULL);
}

bool ipc_enviar_a(int socket, t_paquete* paquete) {
    int bytes = paquete->header->payload_size + sizeof(t_header);
    void* stream = malloc(bytes);
    int offset = 0;

    memcpy(stream + offset, paquete->header, sizeof(t_header));
    offset += sizeof(t_header);
    memcpy(stream + offset, paquete->payload, paquete->header->payload_size);

    int transmitted_bytes = send(socket, stream, bytes, MSG_NOSIGNAL);

    free(stream);

    return transmitted_bytes == bytes;
}

int ipc_conectarse_a(char *ip, char* puerto) {
    struct addrinfo hints;
    struct addrinfo *server_info;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip, puerto, &hints, &server_info);

    int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

    if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1)
        log_warning(logger, "Fallo al intentar establecer conexion");

    freeaddrinfo(server_info);

    return socket_cliente;
}

void ipc_cerrar(int socket) {
    close(socket);
}

void ipc_crear_gameboy_listener(t_listener_config* config) {
    pthread_t gameboyListener;
    pthread_create(&gameboyListener, NULL, (void*)ipc_escuchar_gameboy, config);
    pthread_detach(gameboyListener);
}

int ipc_enviar_suscripcion(t_tipo_mensaje tipo_mensaje, uint32_t id_suscriptor, uint32_t tiempo_conexion, t_listener_config* config) {
    t_suscripcion* s = suscripcion_crear(tipo_mensaje, id_suscriptor, tiempo_conexion);
    t_paquete* p = paquete_from_suscripcion(s);

    int broker = ipc_enviar_broker(p, config->ip, config->puerto);

    suscripcion_liberar(s);
    paquete_liberar(p);

    return broker;
}

void ipc_suscribirse_a(t_listener_config* config) {
    int error, broker;
    do {
        broker = ipc_enviar_suscripcion(config->tipo_mensaje, config->id, 0, config);
        if (broker != -1) {
            log_info(logger, "Conexion establecida con el Broker");
            error = config->handler(broker);
            log_warning(logger, "Se perdio la conexion con Broker");
        }
        sleep(config->reintento_conexion);
        logs_broker_reintento();
    } while (broker == -1 || error);
}

int ipc_enviar_broker(t_paquete* p, char* ip, char* puerto) {
    int broker = ipc_conectarse_a(ip, puerto);
    bool ok = ipc_enviar_a(broker, p);
    logger_enviado(logger, p);
    if(!ok){
        logs_broker_error();
    }
    return ok ? broker : -1;
}

void ipc_enviar_ack(uint32_t id_suscriptor, uint32_t id_mensaje, int socket) {
    void* ack = ack_crear(id_suscriptor, id_mensaje);
    void* paquete = paquete_from_ack(ack);

    ipc_enviar_a(socket, paquete);
    logger_enviado(logger, paquete);

    ack_liberar(ack);
    paquete_liberar(paquete);
}

static void ipc_escuchar_gameboy(t_listener_config* config) {
    int server = ipc_escuchar_en(config->ip, config->puerto);
    while(1) {
        int cliente = ipc_esperar_cliente(server);
        config->handler(cliente);
    }
}

static void logs_broker_error(){
    log_info(logger, "Error de comunicacion con el Broker - Operacion por Default iniciada");
}

static void logs_broker_reintento(){
    log_info(logger, "Inicio de proceso de reintento de comunicacion con el Broker");
}
