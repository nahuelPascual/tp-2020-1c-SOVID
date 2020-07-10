/*
 * filesystem.c
 *
 *  Created on: 21 jun. 2020
 *      Author: utnso
 */

#include "filesystem.h"

static void validar_punto_montaje();
static void metadata_init();
static void files_init();
static void blocks_init();
static void resetear_punto_montaje(const char*);
static char* crear_dir_from_root(char*);
static char* get_path_from_file(char*, char*);
static t_pokemon_info* parse_pokemon_info(char*);
static void crear_default_metadata_info(FILE*);
static void crear_default_files_info(FILE*);
static void crear_default_pokemon_info(FILE*);

void filesystem_init(){
    validar_punto_montaje();
    metadata_init();
    files_init();
    blocks_init();
}

static void validar_punto_montaje(){
    char* const punto_montaje = config_game_card->punto_montaje_tall_grass;

    if( access(punto_montaje, R_OK|W_OK) != -1 ){
        log_info(logger, "Se encontro el punto de montaje %s y se tienen permisos de lectura y escritura.", punto_montaje);
        resetear_punto_montaje(punto_montaje);
    } else if (access(punto_montaje, F_OK) != -1){
        log_info(logger, "Se encontro el punto de montaje %s pero no se tienen permisos de lectura y/o escritura.", punto_montaje);
    } else {
        log_info(logger, "No se encontro el punto de montaje %s se procede a crearlo...", punto_montaje);
        if (mkdir(punto_montaje, 0777) < 0){
            log_error(logger, "No se pudo crear el punto de montaje %s %s.", punto_montaje, strerror(errno));
            exit(1);
        }
    }
}

static void resetear_punto_montaje(const char* punto_montaje){
    log_debug(logger, "Se procede a resetear el punto de montaje: %s...", punto_montaje);
    FTS *ftsp = NULL;
    FTSENT *curr;
    char *files[] = { (char *) punto_montaje, NULL }; // Cast needed because fts_open() takes a "char * const *"
    // FTS_NOCHDIR  - Avoid changing cwd, which could cause unexpected behavior in multithreaded programs
    // FTS_PHYSICAL - Don't follow symlinks. Prevents deletion of files outside of the specified directory
    // FTS_XDEV     - Don't cross filesystem boundaries
    ftsp = fts_open(files, FTS_NOCHDIR | FTS_PHYSICAL | FTS_XDEV, NULL);
    if (!ftsp){
        log_error(logger, "Error abriendo el punto de montaje: %s %s.", punto_montaje,  strerror(errno));
    }
    while ((curr = fts_read(ftsp))){
        switch (curr->fts_info) {
           case FTS_DP: // este es el evento que lee el directorio en postorden
               if(strcmp(curr->fts_accpath, punto_montaje) != 0) { // al dir. tall-grass no lo borro
                   if (remove(curr->fts_accpath) == 0) {
                       log_debug(logger, "Se elimino el directorio %s correctamente.", curr->fts_path);
                   }else{
                       log_error(logger, "Error eliminando el directorio %s %s.", curr->fts_path, strerror(curr->fts_errno));
                   }
               }
               break;
           case FTS_F:
               if (remove(curr->fts_accpath) == 0) {
                   log_debug(logger, "Se elimino el archivo %s correctamente.", curr->fts_path);
               }else{
                   log_error(logger, "Error eliminando el archivo %s %s.", curr->fts_path, strerror(curr->fts_errno));
               }
               break;
           case FTS_ERR:
               log_error(logger, "Error leyendo: %s %s.",curr->fts_accpath, strerror(curr->fts_errno));
               break;
        }
    }
    fts_close(ftsp);
}

static void metadata_init(){
    char* metadata_dir_path = crear_dir_from_root(METADATA_DIR_NAME);

    char* metadata_info_path = get_path_from_file(metadata_dir_path, METADATA_FILE_NAME);
    abrir_archivo(metadata_info_path, crear_default_metadata_info);

    char* bitmap_file_path = get_path_from_file(metadata_dir_path, BITMAP_FILE_NAME);
    abrir_archivo(bitmap_file_path, NULL);

    free(metadata_dir_path);
    free(metadata_info_path);
    free(bitmap_file_path);
}

static void files_init(){
    char* files_dir_path = crear_dir_from_root(FILES_DIR_NAME);
    char* files_info_path = get_path_from_file(files_dir_path, METADATA_FILE_NAME);
    abrir_archivo(files_info_path, crear_default_files_info);
    free(files_dir_path);
    free(files_info_path);
}

static void blocks_init(){
    char* blocks_dir_path = crear_dir_from_root(BLOCKS_DIR_NAME);
    free(blocks_dir_path);
}

static char* crear_dir_from_root(char* dir_name){
    char* dir_path = get_path_from_file(config_game_card->punto_montaje_tall_grass, dir_name);
    abrir_directorio(dir_path);
    return dir_path;
}

t_pokemon_info* check_pokemon_info(char* pokemon_name){
    string_to_upper(pokemon_name);
    char* files_path = get_path_from_file(config_game_card->punto_montaje_tall_grass, FILES_DIR_NAME);
    char* pokemon_dir_path = get_path_from_file(files_path, pokemon_name);
    abrir_directorio(pokemon_dir_path);

    char* pokemon_info_path = get_path_from_file(pokemon_dir_path, METADATA_FILE_NAME);
    abrir_archivo(pokemon_info_path, crear_default_pokemon_info);

    t_pokemon_info* pokemon_info = parse_pokemon_info(pokemon_info_path);

    free(files_path);
    free(pokemon_dir_path);
    return pokemon_info;
}

static void crear_default_metadata_info(FILE* metadata_file){
    fprintf(metadata_file, "%s=%d\n", METADATA_INFO_BLOCK_SIZE, config_game_card->block_size);
    fprintf(metadata_file, "%s=%d\n", METADATA_INFO_BLOCKS, config_game_card->blocks);
    fprintf(metadata_file, "%s=%s", METADATA_INFO_MAGIC_NUMBER, config_game_card->magic_number);
}

static void crear_default_files_info(FILE* files_info){
    fprintf(files_info, "%s=Y\n", FILES_INFO_DIRECTORY);
}

static void crear_default_pokemon_info(FILE* pokemon_file){
    fprintf(pokemon_file, "%s=N\n", POKEMON_INFO_DIRECTORY);
    fprintf(pokemon_file, "%s=0\n", POKEMON_INFO_SIZE);
    fprintf(pokemon_file, "%s=[]\n", POKEMON_INFO_BLOCKS);
    fprintf(pokemon_file, "%s=N", POKEMON_INFO_OPEN);
}

static t_pokemon_info* parse_pokemon_info(char* pokemon_info_path){
    t_pokemon_info* pokemon_info = malloc(sizeof(t_pokemon_info));
    t_config* pokemon_config = config_create(pokemon_info_path);

    pokemon_info->is_directory = string_equals_ignore_case(config_get_string_value(pokemon_config, POKEMON_INFO_DIRECTORY), TRUE);
    pokemon_info->size = config_get_int_value(pokemon_config, POKEMON_INFO_SIZE);
    pokemon_info->blocks = config_get_array_value(pokemon_config, POKEMON_INFO_BLOCKS);
    pokemon_info->is_open = string_equals_ignore_case(config_get_string_value(pokemon_config, POKEMON_INFO_OPEN), TRUE);
    pokemon_info->file_path = pokemon_info_path;
    return pokemon_info;
}

void abrir_directorio(char* dir_path){
    if( access( dir_path, F_OK ) != -1 ){
        log_debug(logger, "El directorio %s ya existe.", dir_path);
     } else {
         log_debug(logger, "El directorio %s no existe.", dir_path);
         log_debug(logger, "Creando directorio %s...", dir_path);
         if (mkdir(dir_path, 0777) < 0){
             log_error(logger, "No se pudo crear el directorio %s %s.", dir_path, strerror(errno));
             exit(1);
         }
     }
}

FILE* abrir_archivo(char* file_path, void (*file_handler)(FILE*)){
    bool exists = true;
    if( access( file_path, F_OK ) != -1 ){
         log_debug(logger, "El archivo %s ya existe.", file_path);
    } else {
        log_debug(logger, "El archivo %s no existe.", file_path);
        log_debug(logger, "Creando archivo %s...", file_path);
        exists = false;
    }
    FILE* file = fopen(file_path ,"a");
    if(!exists && file_handler)
        file_handler(file);
    fclose(file);
    return file;
}

static char* get_path_from_file(char* path, char* file){
    char* file_path = string_new();
    string_append(&file_path, path);
    string_append(&file_path, PATH_SEPARATOR);
    string_append(&file_path, file);
    return file_path;
}
