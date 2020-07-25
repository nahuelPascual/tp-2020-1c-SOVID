//
// Created by utnso on 17/07/20.
//

#include "sincronizacion.h"

static t_dictionary* semaforos;

void sincronizacion_init() {
    semaforos = dictionary_create();
    pthread_mutex_init(&mx_semaforos, NULL);
}

void sincronizacion_lock(char* pokemon) {
    pthread_mutex_lock(&mx_semaforos);
    if (!dictionary_has_key(semaforos, pokemon)) {
      pthread_mutex_t m;
      pthread_mutex_init(&m, NULL);
      pthread_mutex_t* sem = malloc(sizeof(pthread_mutex_t));
      memcpy(sem, &m, sizeof(pthread_mutex_t));
      dictionary_put(semaforos, pokemon, sem);
    }
    pthread_mutex_t* mutex = dictionary_get(semaforos, pokemon);
    pthread_mutex_unlock(&mx_semaforos);
    pthread_mutex_lock(mutex);

}

void sincronizacion_unlock(char* pokemon) {
    pthread_mutex_lock(&mx_semaforos);
    pthread_mutex_t* mx = dictionary_get(semaforos, pokemon);
    pthread_mutex_unlock(&mx_semaforos);
    pthread_mutex_unlock(mx);
}
