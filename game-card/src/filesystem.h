/*
 * filesystem.h
 *
 *  Created on: 21 jun. 2020
 *      Author: utnso
 */

#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fts.h>
#include <sys/stat.h>
#include <commons/string.h>
#include <commons/bitarray.h>
#include <commons/collections/list.h>
#include <delibird-commons/model/mensaje.h>
#include "configuration.h"
#include "sincronizacion.h"

extern FILE* bitmap_file;
extern t_list* pokemon_info;
extern t_bitarray* bitarray;

#define METADATA_DIR_NAME "Metadata"
#define FILES_DIR_NAME "Files"
#define BLOCKS_DIR_NAME "Blocks"
#define METADATA_FILE_NAME "Metadata"
#define BITMAP_FILE_NAME "Bitmap"
#define POKEMON_INFO_DIRECTORY "DIRECTORY"
#define POKEMON_INFO_SIZE "SIZE"
#define POKEMON_INFO_BLOCKS "BLOCKS"
#define POKEMON_INFO_OPEN "OPEN"
#define METADATA_INFO_BLOCK_SIZE "BLOCK_SIZE"
#define METADATA_INFO_BLOCKS "BLOCKS"
#define METADATA_INFO_MAGIC_NUMBER "MAGIC_NUMBER"
#define FILES_INFO_DIRECTORY "DIRECTORY"
#define PATH_SEPARATOR "/"
#define COMMA_SEPARATOR ","
#define KEY_SEPARATOR "-"
#define FILE_EXTENSION ".bin"
#define TRUE "Y"
#define FALSE "N"

typedef struct {
    char* name;
    bool is_directory;
    int size;
    bool is_open;
    t_list* blocks; //lista de char* para ptros de bloques
    char* file_path;
    t_config* config;
} t_pokemon_info;

typedef struct {
    char* path;
    FILE* file;
} t_path;

typedef struct {
    int cantidad_coordenadas;
    t_list* coordenadas;
} t_localized_info;

extern t_path* blocks_dir_path;
extern t_path* files_dir_path;
extern t_path* metadata_dir_path;

void filesystem_init();
void fs_new_pokemon (t_new_pokemon*);
bool fs_catch_pokemon(t_catch_pokemon*);
bool check_file_open(t_pokemon_info*);
t_localized_info* fs_get_pokemon(t_get_pokemon*);
void liberar_t_path(t_path*);

#endif /* FILESYSTEM_H_ */
