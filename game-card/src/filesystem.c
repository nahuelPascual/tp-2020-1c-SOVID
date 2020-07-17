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
static void resetear_punto_montaje(const char* punto_montaje, void(*closure_dir)(FTSENT*), void(*closure_file)(FTSENT*));
static t_path* crear_dir_from_root(char*);
static char* get_path_from_file(char*, char*, char*);
static t_pokemon_info* parse_pokemon_info(char*, char*);
static void crear_default_metadata_info(FILE*);
static void crear_default_files_info(FILE*);
static void crear_default_pokemon_info(FILE*);
static void crear_default_bitarray_info(FILE*);
static char* string_from_list(t_list*);
static char* get_block_key(t_coord*);
static int get_block_size(int, t_new_pokemon*, bool, char*);
static int get_first_empty_block();
static t_config* get_block_data(int);
static void liberar_pokemon_info(t_pokemon_info*);
static t_pokemon_info* crear_pokemon_info(char*);
static t_pokemon_info* check_pokemon_info(char*);
static void save_pokemon_info(t_pokemon_info*);
static void crear_archivo(char*, void (*func)(FILE*));
static FILE* abrir_directorio(char*);
static FILE* abrir_archivo(char*, void (*func)(FILE*));
static void check_file_open(t_pokemon_info*);
static int get_last_block_size(t_list* blocks);
static void resetear_bitmap(FTSENT*);
static void eliminar_dir(FTSENT*);
static void eliminar_file(FTSENT*);
static void configurar_bitmap(void* element);

t_path* blocks_dir_path;
t_path* files_dir_path;
t_path* metadata_dir_path;
FILE* bitmap_file;
t_bitarray* bitarray;
t_list* existing_blocks;

void filesystem_init(){
    validar_punto_montaje();
    files_init();
    metadata_init();
    blocks_init();
}

static void validar_punto_montaje(){
    char* const punto_montaje = config_game_card->punto_montaje_tall_grass;

    if( access(punto_montaje, R_OK|W_OK) != -1 ){
        log_info(logger, "Se encontro el punto de montaje %s y se tienen permisos de lectura y escritura.", punto_montaje);
        //resetear_punto_montaje(punto_montaje, eliminar_dir, eliminar_file); // esto sirve para las pruebas, comentar linea en PROD ya que no deberiamos eliminar el FS
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

static void eliminar_dir(FTSENT* dir_path){
    if(remove(dir_path->fts_accpath) == 0)
        log_debug(logger, "Se elimino el directorio %s correctamente.", dir_path->fts_path);
    else
        log_error(logger, "Error eliminando el directorio %s %s.", dir_path->fts_path, strerror(dir_path->fts_errno));
}

static void eliminar_file(FTSENT* file_path){
    if(remove(file_path->fts_accpath) == 0)
        log_debug(logger, "Se elimino el directorio %s correctamente.", file_path->fts_path);
    else
        log_error(logger, "Error eliminando el directorio %s %s.", file_path->fts_path, strerror(file_path->fts_errno));
}

static void resetear_bitmap(FTSENT* file_path){
    //estoy adentro del archivo
    t_pokemon_info* pokemon_info = parse_pokemon_info(NULL, file_path->fts_accpath);
    list_add_all(existing_blocks, pokemon_info->blocks);
}

static void resetear_punto_montaje(const char* punto_montaje, void(*closure_dir)(FTSENT*), void(*closure_file)(FTSENT*)){
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
                   closure_dir(curr);
               }
               break;
           case FTS_F:
               closure_file(curr);
               break;
           case FTS_ERR:
               log_error(logger, "Error leyendo: %s %s.",curr->fts_accpath, strerror(curr->fts_errno));
               break;
        }
    }
    fts_close(ftsp);
}

static void metadata_init(){
    metadata_dir_path = crear_dir_from_root(METADATA_DIR_NAME);

    char* metadata_info_path = get_path_from_file(metadata_dir_path->path, METADATA_FILE_NAME, FILE_EXTENSION);
    crear_archivo(metadata_info_path, crear_default_metadata_info);

    char* bitmap_file_path = get_path_from_file(metadata_dir_path->path, BITMAP_FILE_NAME, FILE_EXTENSION);
    bitmap_file = abrir_archivo(bitmap_file_path, crear_default_bitarray_info);

    existing_blocks = list_create();
    resetear_punto_montaje(files_dir_path->path, NULL, resetear_bitmap);
    list_iterate(existing_blocks, configurar_bitmap);

    free(metadata_info_path);
    free(bitmap_file_path);
}

static void configurar_bitmap(void* element){
    bitarray_set_bit(bitarray, atoi(element));
}

static void files_init(){
    files_dir_path = crear_dir_from_root(FILES_DIR_NAME);
    char* files_info_path = get_path_from_file(files_dir_path->path, METADATA_FILE_NAME, FILE_EXTENSION);
    crear_archivo(files_info_path, crear_default_files_info);
    free(files_info_path);
}

static void blocks_init(){
    blocks_dir_path = crear_dir_from_root(BLOCKS_DIR_NAME);
    for(int i = 0; i < config_game_card->blocks; i++){
        char* block_path = string_new();
        char* block_number = string_itoa(i);
        string_append(&block_path, blocks_dir_path->path);
        string_append(&block_path, PATH_SEPARATOR);
        string_append(&block_path, block_number);
        string_append(&block_path, FILE_EXTENSION);
        crear_archivo(block_path, NULL);
        free(block_path);
        free(block_number);
    }
}

static t_pokemon_info* check_pokemon_info(char* pokemon_name){
    t_pokemon_info* pokemon_info = NULL;
    string_to_upper(pokemon_name);
    char* pokemon_dir_path = get_path_from_file(files_dir_path->path, pokemon_name, NULL);

    if( access( pokemon_dir_path, F_OK ) != -1 ){
        char* pokemon_info_path = get_path_from_file(pokemon_dir_path, METADATA_FILE_NAME, FILE_EXTENSION);
        if( access( pokemon_info_path, F_OK ) != -1 ){
            pokemon_info = parse_pokemon_info(pokemon_name, pokemon_info_path);
        }
    }
    free(pokemon_dir_path);
    return pokemon_info;
}

static t_pokemon_info* crear_pokemon_info(char* pokemon_name){
    string_to_upper(pokemon_name);
    char* pokemon_dir_path = get_path_from_file(files_dir_path->path, pokemon_name, NULL);
    abrir_directorio(pokemon_dir_path);

    char* pokemon_info_path = get_path_from_file(pokemon_dir_path, METADATA_FILE_NAME, FILE_EXTENSION);
    crear_archivo(pokemon_info_path, crear_default_pokemon_info);

    t_pokemon_info* pokemon_info = parse_pokemon_info(pokemon_name, pokemon_info_path);

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

static void crear_default_bitarray_info(FILE* bitmap_file){
    int cant_bloques = config_game_card->blocks;
    int bitarray_size = cant_bloques / 8;

    if(cant_bloques % 8 != 0){
        bitarray_size++;
    }
    void* puntero_a_bits = malloc(bitarray_size);
    bitarray = bitarray_create_with_mode(puntero_a_bits, bitarray_size, MSB_FIRST);
    for(int i = 0 ; i < cant_bloques; i++){
        bitarray_clean_bit(bitarray, i); //seteo todos los bits en 0
    }
    fputs(bitarray->bitarray, bitmap_file);
}

static t_pokemon_info* parse_pokemon_info(char* name, char* pokemon_info_path){
    t_pokemon_info* pokemon_info = malloc(sizeof(t_pokemon_info));
    t_config* pokemon_config = config_create(pokemon_info_path);

    pokemon_info->name = name;
    pokemon_info->is_directory = string_equals_ignore_case(config_get_string_value(pokemon_config, POKEMON_INFO_DIRECTORY), TRUE);
    pokemon_info->size = config_get_int_value(pokemon_config, POKEMON_INFO_SIZE);
    t_list* blocks = list_create();
    char** blocks_array = config_get_array_value(pokemon_config, POKEMON_INFO_BLOCKS);
    while(*blocks_array != NULL){
        list_add(blocks, *blocks_array);
        blocks_array++;
    }
    pokemon_info->blocks = blocks;
    pokemon_info->is_open = string_equals_ignore_case(config_get_string_value(pokemon_config, POKEMON_INFO_OPEN), TRUE);
    pokemon_info->file_path = pokemon_info_path;
    pokemon_info->config = pokemon_config;

    return pokemon_info;
}

static void save_pokemon_info(t_pokemon_info* pokemon_info){
    log_debug(logger, "Guardando Info del Pokemon: %s en FS:", pokemon_info->name);

    t_config* pokemon_config = pokemon_info->config;
    char* blocks = string_from_list(pokemon_info->blocks);
    char* is_directory = pokemon_info->is_directory ? TRUE : FALSE;
    char* is_open = pokemon_info->is_open ? TRUE : FALSE;
    char* size = string_itoa(pokemon_info->size);

    config_set_value(pokemon_config, POKEMON_INFO_BLOCKS, blocks);
    config_set_value(pokemon_config, POKEMON_INFO_DIRECTORY, is_directory);
    config_set_value(pokemon_config, POKEMON_INFO_OPEN, is_open);
    config_set_value(pokemon_config, POKEMON_INFO_SIZE, size);

    log_debug(logger, "Blocks: %s", blocks);
    log_debug(logger, "Is_Directory: %s", is_directory);
    log_debug(logger, "Is_Open: %s", is_open);
    log_debug(logger, "Size: %s", size);

    config_save(pokemon_config);

    free(blocks);
    free(size);
}

void fs_new_pokemon (t_new_pokemon* new_pokemon){
   int cantidad_pokemones = 0;
   int block_number = 0;
   t_list* blocks;
   t_config* block_data;
   char* key = get_block_key(new_pokemon->posicion);
   bool existe_posicion = false;
   int new_block_size;

   t_pokemon_info* pokemon_info = crear_pokemon_info(new_pokemon->nombre);
   check_file_open(pokemon_info);

   blocks = pokemon_info->blocks;
   int cant_blocks = list_size(blocks);
   for(; block_number < cant_blocks; block_number++){
       block_data = get_block_data(atoi(list_get(blocks, block_number)));
       existe_posicion = config_has_property(block_data, key);
       if(existe_posicion){ // ya existe la posicion
           cantidad_pokemones = config_get_int_value(block_data, key);
           break;
       }
       config_destroy(block_data);
   }
   new_block_size = get_block_size(cantidad_pokemones, new_pokemon, existe_posicion, key);
   if(!existe_posicion){
       // es una nueva posicion tengo que verificar si entra en el ultimo bloque de datos o tengo que usar uno nuevo
       int espacio_libre_ultimo_bloque = config_game_card->block_size - get_last_block_size(pokemon_info->blocks);         ;
       if(cant_blocks == 0 || (new_block_size > espacio_libre_ultimo_bloque)){ // si no entra en ultimo bloque o es el primer new del pokemon
           block_number = get_first_empty_block();
           block_data = get_block_data(block_number);
           bitarray_set_bit(bitarray, block_number);
           fputs(bitarray->bitarray, bitmap_file); // guardo en disco el nuevo bitmap con el bloque ocupado
           list_add(pokemon_info->blocks, string_itoa(block_number)); //actualizo mis ptros a bloques en pokemon_info
       } else{
          char* last_block_number = list_get(pokemon_info->blocks, --cant_blocks);
          block_data = get_block_data(atoi(last_block_number));
       }
    }else{
        //TODO: si existe la posicion tengo que verificar que el nuevo quantity no provoque un mov. de info de bloque por superar el tamaño
    }
    pokemon_info->size += new_block_size;
    pokemon_info->is_open = false;
    save_pokemon_info(pokemon_info);

    char* new_cantidad_pokemon = string_itoa(cantidad_pokemones + new_pokemon->cantidad);
    config_set_value(block_data, key, new_cantidad_pokemon);

    sleep(config_game_card->tiempo_retardo_operacion);

    config_save(block_data);
    config_destroy(block_data);
    free(key);
    free(new_cantidad_pokemon);
    liberar_pokemon_info(pokemon_info);
}

static int get_first_empty_block(){
    int block = 0;
    while(bitarray_test_bit(bitarray, block)){ // mientras devuelva 1 sigo recorriendo el bitmap.
        block++;
   }
    return block;
}

static int get_last_block_size(t_list* blocks) {
    t_config* block_data = get_block_data(atoi(list_get(blocks, list_size(blocks)-1)));
    int size = 0;
    void contar(char* key, void* value){
        size += string_length(key) + string_length(value) + 2;
    }
    dictionary_iterator(block_data->properties, contar);
    return size;
}

static int get_block_size(int cantidad_pokemones, t_new_pokemon* new_pokemon, bool existe_posicion, char* key){
    //TODO: esto es un asco: uso el proxy de que 1 char en disco es = 1 byte pero podria hacerlo mejor usando fseek y obteniendo el tamaño del file.
    int block_size;
    char* current_cantidad_pokemones_str = string_itoa(cantidad_pokemones);
    cantidad_pokemones += new_pokemon->cantidad;
    char* new_cantidad_pokemones_str = string_itoa(cantidad_pokemones);
    if(!existe_posicion){
       block_size = string_length(key) + string_length(new_cantidad_pokemones_str) + 2;  //sumo 2 uno por el "=" y otro por el /n
    } else{ // sumo los bytes del size solo si paso de unidad.
       block_size = (string_length(new_cantidad_pokemones_str) - string_length(current_cantidad_pokemones_str));
    }
    free(current_cantidad_pokemones_str);
    free(new_cantidad_pokemones_str);
    return block_size;
}

static char* get_block_key(t_coord* coord){
    char* key = string_new();
    char* x_str = string_itoa(coord->x);
    char* y_str = string_itoa(coord->y);
    string_append(&key, x_str);
    string_append(&key, KEY_SEPARATOR);
    string_append(&key, y_str);
    free(x_str);
    free(y_str);
    return key;
}

static char* string_from_list(t_list* list){
    char* string = string_new();
    string_append(&string, "[");
    for(int i = 0; i < list_size(list); i++){
        string_append(&string, list_get(list, i));
        if(list_size(list) > 1 && (list_size(list) != i+1))
            string_append(&string, COMMA_SEPARATOR);
    }
    string_append(&string, "]");
    return string;
}

static t_path* crear_dir_from_root(char* dir_name){
    t_path* path_info = malloc(sizeof(t_path));
    char* dir_path = get_path_from_file(config_game_card->punto_montaje_tall_grass, dir_name, NULL);
    FILE* dir = abrir_directorio(dir_path);
    path_info->file = dir;
    path_info->path = dir_path;
    return path_info;
}

static FILE* abrir_directorio(char* dir_path){
    if( access( dir_path, F_OK ) != -1 ){
        //log_debug(logger, "El directorio %s ya existe.", dir_path);
     } else {
         log_debug(logger, "El directorio %s no existe.", dir_path);
         log_debug(logger, "Creando directorio %s...", dir_path);
         if (mkdir(dir_path, 0777) < 0){
             log_error(logger, "No se pudo crear el directorio %s %s.", dir_path, strerror(errno));
             exit(1);
         }
     }
    return fopen(dir_path ,"a");
}

static void crear_archivo(char* file_path, void (*file_handler)(FILE*)){
    FILE* file = abrir_archivo(file_path, file_handler);
    fclose(file);
}

static FILE* abrir_archivo(char* file_path, void (*file_handler)(FILE*)){
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
    return file;
}

static char* get_path_from_file(char* path, char* file_name, char* file_extension){
    char* file_path = string_new();
    string_append(&file_path, path);
    string_append(&file_path, PATH_SEPARATOR);
    string_append(&file_path, file_name);
    if(file_extension)
        string_append(&file_path, file_extension);
    return file_path;
}

bool fs_catch_pokemon(t_catch_pokemon* catch_pokemon){
    bool is_caught = false;
    int block = 0;
    char* cant_pokemones_str;
    char* nueva_cant_pokemones_str = NULL;
    t_config* block_data;
    t_pokemon_info* pokemon_info = check_pokemon_info(catch_pokemon->nombre);
    if(pokemon_info){
        char* key = get_block_key(catch_pokemon->posicion);
        t_list* blocks = pokemon_info->blocks;
        for(; block < list_size(blocks); block++){
            block_data = get_block_data(atoi(list_get(blocks, block)));
            is_caught = config_has_property(block_data, key);
            if(is_caught){
                int block_size = 0;
                // si era el ultimo pokemon elimino la entrada de lo contrario solo resto en 1 la cantidad.
                int cant_pokemones = config_get_int_value(block_data, key);
                cant_pokemones_str = string_itoa(cant_pokemones);
                if(cant_pokemones == 1){ //es el ultimo pokemon elimino la key del bloque
                    config_remove_key(block_data, key);
                    // si es la ultima entrada libero el bloque del bitmap y actualizo el ptro del FCB.
                    if(dictionary_is_empty(block_data->properties)){
                        bool find_block (void* element){
                            bool found = false;
                            if(block == atoi(element))
                                found = true;
                            return found;
                        }
                        bitarray_clean_bit(bitarray, block);//pongo el bit en cero
                        fputs(bitarray->bitarray, bitmap_file);
                        list_remove_by_condition(pokemon_info->blocks, find_block);
                    }
                    block_size = string_length(key) + string_length(cant_pokemones_str) + 2; // sumo el "=" y el salto de linea.
                }else{
                    //calculo el nuevo size del archivo y actualizo la cantidad de pokemons en el bloque de datos.
                    nueva_cant_pokemones_str = string_itoa(--cant_pokemones);
                    if(string_length(cant_pokemones_str) - string_length(nueva_cant_pokemones_str) > 0){ //TODO usando fseek es mas facil saber los bytes del archivo
                        block_size = 1; // ej: tenia 100 y al restarle 1 queda en 99, le resto un byte
                    }
                    config_set_value(block_data, key, nueva_cant_pokemones_str);
                }
                pokemon_info->size -= block_size;
                save_pokemon_info(pokemon_info);
                sleep(config_game_card->tiempo_retardo_operacion); // solo uso el retardo para operar los bloques no con el FCB, decision de diseño.
                config_save(block_data);
                free(cant_pokemones_str);
                //free(nueva_cant_pokemones_str);
                break;
            }
            config_destroy(block_data);
        }
        free(key);
        liberar_pokemon_info(pokemon_info);
    }
    if(nueva_cant_pokemones_str)
            free(nueva_cant_pokemones_str);
    return is_caught;
}

t_localized_info* fs_get_pokemon(t_get_pokemon* get_pokemon){
    t_localized_info* localized_info = malloc(sizeof(t_localized_info));
    localized_info->cantidad_coordenadas = 0; //TODO: verificar con Broker como espera esta info cuando no existe el pokemon
    localized_info->coordenadas = list_create();
    t_config* block_data;
    int block = 0;
    t_pokemon_info* pokemon_info = check_pokemon_info(get_pokemon->nombre);
    if(pokemon_info){
        t_list* blocks = pokemon_info->blocks;
        if(!list_is_empty(blocks)){ // si existe el file del pokemon y su FCB tiene ptros
            void get_coordenadas(char* key, void* value){
                char** coordenadas = string_split(key, "-");
                t_coord* coordenada = malloc(sizeof(t_coord));
                coordenada->x = atoi(*coordenadas);
                coordenadas++;
                coordenada->y = atoi(*coordenadas);
                list_add(localized_info->coordenadas, coordenada);
                localized_info->cantidad_coordenadas++;
            }
            for(; block < list_size(blocks); block++){
                block_data = get_block_data(atoi(list_get(blocks, block)));
                dictionary_iterator(block_data->properties, get_coordenadas);
                config_destroy(block_data);
            }
        }
        liberar_pokemon_info(pokemon_info);
    }
    return localized_info;
}

static t_config* get_block_data(int block){
   char* block_str = string_itoa(block);
   char* block_file_path = get_path_from_file(blocks_dir_path->path, block_str, FILE_EXTENSION);
   free(block_str);
   return config_create(block_file_path);
}

static void liberar_pokemon_info(t_pokemon_info* pokemon_info){
    list_destroy_and_destroy_elements(pokemon_info->blocks, free);
    config_destroy(pokemon_info->config);
    free(pokemon_info->file_path);
    free(pokemon_info);
}

void liberar_t_path(t_path* t_path){
    fclose(t_path->file);
    free(t_path->file);
    free(t_path->path);
    free(t_path);
}

static void check_file_open(t_pokemon_info* pokemon_info){//TODO: usar diccionario de semaforos uno por tipo de pokemon
    if(!pokemon_info->is_open){
       log_info(logger, "El archivo %s no se encuentra abierto", pokemon_info->file_path);
       pokemon_info->is_open = true;
       save_pokemon_info(pokemon_info); // guardo la info en la FCB. TODO: deberia tener un memoria las FCB con acceso directo a la info.
    }else{
       log_error(logger, "El archivo %s se encuentra abierto.", pokemon_info->file_path);
       //TODO: implementar politica de reintentos utilizando config_game_card->tiempo_reintento_operacion.
    }
}
