/*
 * filesystem.h
 *
 *  Created on: 21 jun. 2020
 *      Author: utnso
 */

#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fts.h>
#include <sys/stat.h>
#include <commons/string.h>
#include "configuration.h"

#define METADATA_DIR_NAME "Metadata"
#define FILES_DIR_NAME "Files"
#define BLOCKS_DIR_NAME "Blocks"
#define METADATA_FILE_NAME "Metadata.bin"
#define BITMAP_FILE_NAME "Bitmap.bin"
#define POKEMON_INFO_DIRECTORY "DIRECTORY"
#define POKEMON_INFO_SIZE "SIZE"
#define POKEMON_INFO_BLOCKS "BLOCKS"
#define POKEMON_INFO_OPEN "OPEN"
#define METADATA_INFO_BLOCK_SIZE "BLOCK_SIZE"
#define METADATA_INFO_BLOCKS "BLOCKS"
#define METADATA_INFO_MAGIC_NUMBER "MAGIC_NUMBER"
#define FILES_INFO_DIRECTORY "DIRECTORY"
#define PATH_SEPARATOR "/"
#define TRUE "Y"
#define FALSE "N"

typedef struct {
    bool is_directory;
    int size;
    bool is_open;
    char** blocks;
    char* file_path;
} t_pokemon_info;

void filesystem_init();
void abrir_directorio(char*);
FILE* abrir_archivo(char*, void (*func)(FILE*));
t_pokemon_info* check_pokemon_info(char*);

#endif /* FILESYSTEM_H_ */
