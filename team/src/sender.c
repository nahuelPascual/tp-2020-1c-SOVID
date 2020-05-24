//
// Created by utnso on 9/05/20.
//

#include "sender.h"

t_list* capturas_pendientes;

void enviar_get_pokemon(char* pokemon, void* cantidad) {
	t_get_pokemon* mensaje = mensaje_crear_get_pokemon(pokemon);
	t_paquete* paquete = paquete_from_get_pokemon(mensaje);

	int broker = ipc_conectarse_a("127.0.0.1", "8081"); // TODO levantar de config
	ipc_enviar_a(broker, paquete);

	mensaje_liberar_get_pokemon(mensaje);
	paquete_liberar(paquete);
	ipc_cerrar(broker);
}

t_captura* get_mensaje_enviado(int id_mensaje) {
    for (int i=0 ; i>list_size(capturas_pendientes) ; i++) {
        t_captura* captura = list_get(capturas_pendientes, i);
        if (captura->id_mensaje == id_mensaje) {
            return (t_captura*) list_remove(capturas_pendientes, i);
        }
    }
    return NULL;
}
