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
static void recorrer_arbol_directorio(const char* punto_montaje, void(*closure_dir)(FTSENT*), void(*closure_file)(FTSENT*));
static t_path* crear_dir_from_root(char*);
static char* get_path_from_file(char*, char*, char*);
static t_pokemon_info* parse_pokemon_info(char*, char*);
static void crear_default_metadata_info(FILE*);
static void crear_default_files_info(FILE*);
static void crear_default_pokemon_info(FILE*);
static void crear_default_bitarray_info(FILE*);
static char* string_from_list(t_list*);
static char* get_block_key(t_coord*);
static int get_first_empty_block();
static void liberar_pokemon_info(t_pokemon_info*);
static t_pokemon_info* crear_pokemon_info(char*);
static t_pokemon_info* check_pokemon_info(char*);
static void save_pokemon_info(t_pokemon_info*);
static void crear_archivo(char*, void (*func)(FILE*));
static FILE* abrir_directorio(char*);
static FILE* abrir_archivo(char*, void (*func)(FILE*), bool);
static void resetear_bitmap(FTSENT*);
static void configurar_bitmap(void* element);
static void close_file(t_pokemon_info*);
static t_list* parse_blocks_info(t_list*, t_pokemon_info*);
static char* get_block_data_from_list(t_list*, t_pokemon_info*);

t_path* blocks_dir_path;
t_path* files_dir_path;
t_path* metadata_dir_path;
FILE* bitmap_file;
t_bitarray* bitarray;
t_list* existing_blocks;
pthread_mutex_t lock;

void filesystem_init(){
    pthread_mutex_init(&lock, NULL);
    validar_punto_montaje();
    files_init();
    metadata_init();
    blocks_init();
    sincronizacion_init();
}

static void validar_punto_montaje(){
    char* const punto_montaje = config_game_card->punto_montaje_tall_grass;

    if( access(punto_montaje, R_OK|W_OK) != -1 ){
        log_info(logger, "Se encontro el punto de montaje %s y se tienen permisos de lectura y escritura.", punto_montaje);
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

static void recorrer_arbol_directorio(const char* punto_montaje, void(*closure_dir)(FTSENT*), void(*closure_file)(FTSENT*)){
    log_debug(logger, "Recorriendo arbol de directorio: %s...", punto_montaje);
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
                   if(closure_dir)
                       closure_dir(curr);
               }
               break;
           case FTS_F:
               if(closure_file)
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
    bitmap_file = abrir_archivo(bitmap_file_path, crear_default_bitarray_info, true);


    existing_blocks = list_create();
    recorrer_arbol_directorio(files_dir_path->path, NULL, resetear_bitmap);
    list_iterate(existing_blocks, configurar_bitmap);

    free(metadata_info_path);
    free(bitmap_file_path);
}

static void resetear_bitmap(FTSENT* file_path){
    if(!string_equals_ignore_case(file_path->fts_accpath, get_path_from_file(files_dir_path->path, METADATA_FILE_NAME, FILE_EXTENSION))){ // ignoro el archivo Files/Metadata.bin
        log_debug(logger, "Actualizando bitmap con file %s.", file_path->fts_accpath);
        t_pokemon_info* pokemon_info = parse_pokemon_info(NULL, file_path->fts_accpath);
        list_add_all(existing_blocks, pokemon_info->blocks);
    }
}

static void configurar_bitmap(void* element){
    int bloque = atoi(element);
    log_debug(logger, "Seteando bloque de datos %i como ocupado.", bloque);
    bitarray_set_bit(bitarray, bloque);
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

    do {
        sincronizacion_lock(pokemon_name);
        if( access( pokemon_dir_path, F_OK ) != -1 ){
            char* pokemon_info_path = get_path_from_file(pokemon_dir_path, METADATA_FILE_NAME, FILE_EXTENSION);
            if( access( pokemon_info_path, F_OK ) != -1 ){
                pokemon_info = parse_pokemon_info(pokemon_name, pokemon_info_path);
            }
        }
    } while (pokemon_info && check_file_open(pokemon_info));
    sincronizacion_unlock(pokemon_name);

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

int fs_new_pokemon (t_new_pokemon* new_pokemon){
   check_pokemon_info(new_pokemon->nombre);

   char* key = get_block_key(new_pokemon->posicion);

   t_pokemon_info* pokemon_info = crear_pokemon_info(new_pokemon->nombre);
   t_list* blocks_ptrs = pokemon_info->blocks;
   t_list* block_info_list = parse_blocks_info(blocks_ptrs, pokemon_info);

   //busco si el pokemon ya existe y actualizo su quantity en la lista
   bool existe_pokemon = false;
   for(int i = 0; i < list_size(block_info_list); i++){
       char* element = list_get(block_info_list, i);
       char** keyAndValue = string_n_split(element, 2, "=");
       if(string_equals_ignore_case(keyAndValue[0], key)){
           existe_pokemon = true;
           log_info(logger, "El pokemon %s ya existe en la posicion %d,%d se le suma cantidad = %d...", new_pokemon->nombre, new_pokemon->posicion->x, new_pokemon->posicion->y, new_pokemon->cantidad);
           int nueva_cantidad_pokemones = new_pokemon->cantidad + atoi(keyAndValue[1]);
           char* newKeyAndValue = string_new();
           string_append(&newKeyAndValue, keyAndValue[0]);
           string_append(&newKeyAndValue, "=");
           char* nueva_cant_str = string_itoa(nueva_cantidad_pokemones);
           string_append(&newKeyAndValue, nueva_cant_str);
           list_remove(block_info_list, i);
           list_add_in_index(block_info_list, i, newKeyAndValue);
           break;
      }
      free(keyAndValue[0]);
      free(keyAndValue);
   }
   if(!existe_pokemon){
       char* new_pokemon_str = string_new();
       char* coordenada_x = string_itoa(new_pokemon->posicion->x);
       char* coordenada_y = string_itoa(new_pokemon->posicion->y);
       char* cantidad = string_itoa(new_pokemon->cantidad);
       log_info(logger, "El pokemon %s no existe en la posicion: %s,%s se agrega con cantidad = %d...", new_pokemon->nombre, coordenada_x, coordenada_y, new_pokemon->cantidad);
       string_append(&new_pokemon_str,coordenada_x );
       string_append(&new_pokemon_str, KEY_SEPARATOR);
       string_append(&new_pokemon_str, coordenada_y);
       string_append(&new_pokemon_str, "=");
       string_append(&new_pokemon_str, cantidad);
       list_add(block_info_list, new_pokemon_str);
   }
   int block_size = config_game_card->block_size;

   // convierto la lista actualizada con la info del nuevo pokemon a un char*
   char* block_data = get_block_data_from_list(block_info_list, pokemon_info);

   // calculo la cantidad total de data en bytes
   int cant_block_bytes = string_length(block_data);

   //calculo la nueva cantidad de bloques a utilizar
   int cant_blocks_necesarios = cant_block_bytes / block_size;
   if(cant_block_bytes % block_size != 0){//fragmentacion interna en el ultimo ptro.
       cant_blocks_necesarios++;
   }
   if(cant_blocks_necesarios > list_size(blocks_ptrs)){ // para guardar el new pokemon necesito un nuevo bloque
       int cant_blocks_faltantes = cant_blocks_necesarios - list_size(blocks_ptrs);
       pthread_mutex_lock(&lock);
       for(int i = 0; i < cant_blocks_faltantes; i++){
           int new_block_number = get_first_empty_block();
           if(new_block_number == -1){
            log_error(logger, "No tengo mas bloques libres. No se puede agregar el Pokemon: %s el FS esta lleno.", new_pokemon->nombre);
            pthread_mutex_unlock(&lock);
            close_file(pokemon_info);
            liberar_pokemon_info(pokemon_info);
            return -1;
           }
           bitarray_set_bit(bitarray, new_block_number);
           fputs(bitarray->bitarray, bitmap_file); // guardo en disco el nuevo bitmap con el bloque ocupado
           list_add(pokemon_info->blocks, string_itoa(new_block_number)); //actualizo mis ptros a bloques en pokemon_info
       }
       pthread_mutex_unlock(&lock);
   }
   //elimino todos los blocks del FS
   void remove_block_file(void* element){
       char* block_number = (char*) element;
       char* path = get_path_from_file(blocks_dir_path->path, block_number,FILE_EXTENSION);
       remove(path);
   }
   list_iterate(blocks_ptrs, remove_block_file);

   int offset = 0;
   void save_file_info(FILE* file){
       char* values = string_substring(block_data, offset, block_size);
       offset += block_size;
       fprintf(file, "%s", values);
       fflush(file);
       free(values);
   }
   //recorro los bloques y en cada uno copio los primeros n bytes de buffer
   for(int i = 0; i < list_size(blocks_ptrs); i++){
       char* block_number = list_get(blocks_ptrs, i);
       char* path = get_path_from_file(blocks_dir_path->path, block_number, FILE_EXTENSION);
       crear_archivo(path, save_file_info);
       free(path);
   }

   pokemon_info->size = cant_block_bytes;
   close_file(pokemon_info);
   liberar_pokemon_info(pokemon_info);
   free(key);
   list_destroy_and_destroy_elements(block_info_list, free);

   return 0;
}

static char* get_block_data_from_list(t_list* block_info_list, t_pokemon_info* pokemon_info){
    int size = 0;
    for(int i = 0; i < list_size(block_info_list); i++) {
        char* element = list_get(block_info_list, i);
        size += string_length(element) + 1;
    }

    char* buffer = calloc(1, size);//malloc(size);//calloc(1, pokemon_info->size + 1);
      int offset = 0;
      for(int i = 0; i < list_size(block_info_list); i++){
          char* element = list_get(block_info_list, i);
          strcpy(buffer + offset, element);
          offset += string_length(element);
          if(i != (list_size(block_info_list) - 1)){ // la ultima posicion no lleva \n
              strcpy(buffer + offset, "\n");
              offset++;
          }
      }
      return buffer;
}

static int get_first_empty_block(){
    int block = -1;
    int cant_blocks_total = config_game_card->blocks;
    for(int i = 0; i < cant_blocks_total; i++){ // me fijo los primeros cant_blocks_total bits del array que siempre es potencia de 2.
        if(!bitarray_test_bit(bitarray, i)){// si el bit es != 1 salgo del loop
            block = i;
            break;
        }
    }
    return block;
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
    FILE* file = abrir_archivo(file_path, file_handler, false);
    fclose(file);
}

static FILE* abrir_archivo(char* file_path, void (*file_handler)(FILE*), bool force_handler){
    bool exists = true;
    if( access( file_path, F_OK ) != -1 ){
         log_debug(logger, "El archivo %s ya existe.", file_path);
    } else {
        log_debug(logger, "El archivo %s no existe.", file_path);
        log_debug(logger, "Creando archivo %s...", file_path);
        exists = false;
    }
    FILE* file = fopen(file_path ,"a");
    if((!exists && file_handler) || force_handler)
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
    t_pokemon_info* pokemon_info = check_pokemon_info(catch_pokemon->nombre);
    if(pokemon_info){
        char* coordenada_a_buscar = get_block_key(catch_pokemon->posicion);
        t_list* blocks_ptrs = pokemon_info->blocks;
        t_list* block_info_list = parse_blocks_info(blocks_ptrs, pokemon_info);
        for(int i = 0; i < list_size(block_info_list); i++){
            char* element = list_get(block_info_list, i);
            char** keyAndValue = string_split(element, "=");
            char* coordenada = keyAndValue[0];
            is_caught = string_equals_ignore_case(coordenada, coordenada_a_buscar);
            if(is_caught){
                int cant_pokemones = atoi(keyAndValue[1]);
                if(cant_pokemones == 1){// si era el ultimo pokemon elimino la entrada de lo contrario solo resto en 1 la cantidad.
                    list_remove(block_info_list, i);
                }
                else{
                    list_remove(block_info_list, i);
                    char* new_keyAndValue = string_new();
                    string_append(&new_keyAndValue, keyAndValue[0]);
                    string_append(&new_keyAndValue, "=");
                    string_append(&new_keyAndValue, string_itoa(cant_pokemones - 1));
                    list_add_in_index(block_info_list, i, new_keyAndValue);
                }
                if(list_is_empty(block_info_list)){ // esto pasa cuando hago catch del ultimo pokemon
                    //elimino todos los blocks del FS
                   void remove_block_file(void* element){
                     char* block_number = (char*) element;
                     char* path = get_path_from_file(blocks_dir_path->path, block_number,FILE_EXTENSION);
                     remove(path);
                     crear_archivo(path, NULL); // creo el bloque vacio
                     free(path);
                   }
                   list_iterate(blocks_ptrs, remove_block_file);

                   pthread_mutex_lock(&lock);
                       for(int i = 0; i < list_size(blocks_ptrs); i++){
                           int block_number = atoi(list_get(blocks_ptrs, i));
                           bitarray_clean_bit(bitarray, block_number);
                       }
                       list_clean(pokemon_info->blocks);
                       pokemon_info->size  = 0;
                       fputs(bitarray->bitarray, bitmap_file); // guardo en disco el nuevo bitmap con el bloque ocupado
                   pthread_mutex_unlock(&lock);
                   break;
                }
                int block_size = config_game_card->block_size;

                // convierto la lista actualizada con la info del nuevo pokemon a un char*
                char* block_data = get_block_data_from_list(block_info_list, pokemon_info);

                // calculo la cantidad total de data en bytes
                int cant_block_bytes = string_length(block_data);

                //calculo la nueva cantidad de bloques a utilizar
                int cant_blocks_necesarios = cant_block_bytes / block_size;
                if(cant_block_bytes % block_size != 0){//fragmentacion interna en el ultimo ptro.
                  cant_blocks_necesarios++;
                }
                //elimino todos los blocks del FS
                void remove_block_file(void* element){
                  char* block_number = (char*) element;
                  char* path = get_path_from_file(blocks_dir_path->path, block_number,FILE_EXTENSION);
                  remove(path);
                  free(path);
                }
                list_iterate(blocks_ptrs, remove_block_file);

                if(cant_blocks_necesarios < list_size(blocks_ptrs)){ //si tuve que eliminar un bloque, libero el ultimo y lo marco como libre
                    int cant_blocks_a_eliminar = list_size(blocks_ptrs) - cant_blocks_necesarios;
                    pthread_mutex_lock(&lock);
                        for(int i = 0; i < cant_blocks_a_eliminar; i++){
                            int last_block_number = atoi(list_get(blocks_ptrs, list_size(blocks_ptrs) - 1));
                            bitarray_clean_bit(bitarray, last_block_number);
                            fputs(bitarray->bitarray, bitmap_file); // guardo en disco el nuevo bitmap con el bloque ocupado
                            char* path = get_path_from_file(blocks_dir_path->path, list_get(blocks_ptrs, list_size(blocks_ptrs) - 1), FILE_EXTENSION);
                            crear_archivo(path, NULL); // creo el bloque vacio
                            list_remove(pokemon_info->blocks, list_size(blocks_ptrs) - 1); //actualizo mis ptros a bloques en pokemon_info
                            free(path);
                        }
                    pthread_mutex_unlock(&lock);
                }
                int offset = 0;
                void save_file_info(FILE* file){
                  char* values = string_substring(block_data, offset, block_size);
                  offset += block_size;
                  fprintf(file, "%s", values);
                  fflush(file);
                  free(values);
                }
                //recorro los bloques y en cada uno copio los primeros n bytes de buffer
                for(int i = 0; i < list_size(blocks_ptrs); i++){
                  char* block_number = list_get(blocks_ptrs, i);
                  char* path = get_path_from_file(blocks_dir_path->path, block_number, FILE_EXTENSION);
                  crear_archivo(path, save_file_info);
                  free(path);
                }
                pokemon_info->size = cant_block_bytes;

                free(keyAndValue[0]);
                free(keyAndValue);
                free(block_data);
                break;
            }
        }
        free(coordenada_a_buscar);
        list_destroy_and_destroy_elements(block_info_list, free);
    }
    close_file(pokemon_info);
    liberar_pokemon_info(pokemon_info);

    return is_caught;
}

t_localized_info* fs_get_pokemon(t_get_pokemon* get_pokemon){
    t_localized_info* localized_info = malloc(sizeof(t_localized_info));
    localized_info->cantidad_coordenadas = 0;
    localized_info->coordenadas = list_create();

    t_pokemon_info* pokemon_info = check_pokemon_info(get_pokemon->nombre);
    if(pokemon_info){
        t_list* blocks_ptrs = pokemon_info->blocks;
        if(!list_is_empty(blocks_ptrs)){ // si existe el file del pokemon y su FCB tiene ptros
            t_list* block_info_list = parse_blocks_info(blocks_ptrs, pokemon_info);
            for(int i = 0; i < list_size(block_info_list); i++){
                 char* element = list_get(block_info_list, i);
                 char** keyAndValue = string_split(element, "=");
                 char* key = keyAndValue[0];
                 char** coordenadas = string_split(key, "-");
                 t_coord* coordenada = malloc(sizeof(t_coord));
                 coordenada->x = atoi(*coordenadas);
                 coordenadas++;
                 coordenada->y = atoi(*coordenadas);
                 list_add(localized_info->coordenadas, coordenada);
                 localized_info->cantidad_coordenadas++;
            }
        }
        close_file(pokemon_info);
        liberar_pokemon_info(pokemon_info);
    }

    if (localized_info->coordenadas == 0)
        return NULL;

    return localized_info;
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

static t_list* parse_blocks_info(t_list* block_ptrs, t_pokemon_info* pokemon_info){
    t_list* list = list_create();
    char* buffer = calloc(1, pokemon_info->size + 1);
    off_t offset = 0;
    for(int i= 0; i < list_size(block_ptrs); i++){
        char* block_number = list_get(block_ptrs, i);
        char* path = get_path_from_file(blocks_dir_path->path, block_number, FILE_EXTENSION);
        FILE* file = fopen(path, "r");
        if (file == NULL) {
            return NULL;
        }
        struct stat stat_file;
        stat(path, &stat_file);
        off_t size = stat_file.st_size;
        fread(buffer + offset, size, 1, file);
        offset += size;
        fclose(file);
        free(path);
    }
    char** lines = string_split(buffer, "\n");
    while(*lines != NULL){
        list_add(list, *lines);
        lines ++;
    }
    free(buffer);
    return list;
}

bool check_file_open(t_pokemon_info* pokemon_info){
    bool is_open = pokemon_info->is_open;
    if(is_open){
        sincronizacion_unlock(pokemon_info->name);
        log_warning(logger, "El archivo %s se encuentra abierto", pokemon_info->file_path);
        sleep(config_game_card->tiempo_reintento_operacion);
    }
    else {
        log_info(logger, "El archivo %s no se encuentra abierto", pokemon_info->file_path);
        pokemon_info->is_open = true;
        save_pokemon_info(pokemon_info); // guardo la info en la FCB. TODO: deberia tener un memoria las FCB con acceso directo a la info.
    }
    return is_open;
}

static void close_file(t_pokemon_info* pokemon_info) {
    sincronizacion_lock(pokemon_info->name);
    pokemon_info->is_open = false;
    save_pokemon_info(pokemon_info);
    sleep(config_game_card->tiempo_retardo_operacion);
    sincronizacion_unlock(pokemon_info->name);
}
