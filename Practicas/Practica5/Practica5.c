#include <stdio.h>    // Para operaciones estándar de entrada y salida.
#include <stdlib.h>   // Para utilizar funciones estándar de C.
#include <sys/stat.h> // Para la verificación de directorios y archivos.
#include <ncurses.h>  // Para controlar el aspecto gráfico dentro de la terminal.
#include <string.h>   // Para realizar operaciones con cadenas de caracteres.
#include <locale.h>   // Para poder mostrar acentos y caracteres especiales en la terminal.

// Constantes de operación.
#define LECTURA_ARCHIVO_OK 0
#define LECTURA_ARCHIVO_PARAMETROS_INVALIDOS -1
#define LECTURA_ARCHIVO_ERROR -2
#define LECTURA_ARCHIVO_MUY_GRANDE -3
#define LECTURA_ARCHIVO_SIN_MEMORIA -4
#define LECTURA_ARCHIVO_TAM_BLOQUE 2097152 // El tamaño de lectura de cada "bloque" de información del archivo 2MiB.
#define TAMANIO_INICIAL 2                  // El tamaño inicial del archivo si es un archivo nuevo.
#define CARACTER_VACIO ' '                 // Cuando la pantalla no debe mostrar nada
#define CARACTER_DE_CONTROL ' '            // Cuando existe un caracter de control (\0 o \n), para debuggear más fácil qué está sucediendo.

// Prototipos de funciones.
int leer_archivo_en_buffer(char *, char **, size_t *);                        // Función para leer un archivo y guardarlo en un búffer.
int existe_archivo(char *);                                                   // Función para determinar si existe un archivo.
void editor_de_texto(char *, char **, size_t *, int, int);                    // Función que encapsula la operación del editor.
void calcular_pantalla(int **, int, int, int, int, char **, size_t);          // Función que calcula los contenidos de la pantalla en cada interacción del usuario.
void mostrar_pantalla(int, int, char **, int, int, int **);                   // Función que muestra la pantalla calculada
void mostrar_estadisticas(char *, char **, size_t, int, int, int, int);       // Función que muestra estadísticas en la barra de menú.
void tecla_abajo(int, int **, int *, int *, char **, int *);                  // Función para encapsular la lógica de presionar la tecla "Flecha Abajo"
void tecla_arriba(int, int **, int *, int *, char **, int *);                 // Función para encapsular la lógica de presionar la tecla "Flecha Arriba"
void tecla_izquierda(int *, int *);                                           // Función para encapsular la lógica de presionar la tecla "Flecha Izquierda"
void tecla_derecha(int *, int *, int, size_t, char **);                       // Función para encapsular la lógica de presionar la tecla "Flecha Derecha"
void tecla_borrar(int *, int *, char **, size_t *, int *, int **);            // Función para encapsular la lógica de presionar la tecla "Borrar"
void tecla_esc(char *, char **, size_t);                                      // Función para encapsular la lógica de presionar la tecla "ESC" (salir)
void tecla_cualquiera(char **, size_t *, int, int *, int *, int, int *, int); // Función para encapsular la lógica de presionar cualquier otra tecla.

int main(int argc, char *argv[]){
  char *buffer, *nombre_archivo;         
  int resultado_lectura_archivo, filas_maximas, columnas_maximas, archivo_existente;      
  size_t tamanio_buffer;        

  // Verificamos si obtuvimos el nombre del archivo como parámetro del programa.
  if (argc == 2){
    nombre_archivo = argv[1]; // Asumimos que el segundo parámetro es el nombre del archivo.
  }else{ // Mostramos la información de uso del programa al usuario.
    printf("USO:\n");
    printf("\t%s /ruta/al/archivo/para/editar.txt\n\n", argv[0]);
    return 0;
  }

  archivo_existente = existe_archivo(nombre_archivo); // Verificamos que el archivo exista.

  if (archivo_existente){ // Si el archivo existe leemos su contenido.
    printf("Leyendo '%s' ...\n", nombre_archivo);

    // Leemos el archivo en el búffer.
    resultado_lectura_archivo = leer_archivo_en_buffer(nombre_archivo, &buffer, &tamanio_buffer);

    // Verificamos la lectura del archivo.
    switch (resultado_lectura_archivo){
    case LECTURA_ARCHIVO_OK:
      printf("¡Archivo leído correctamente!\n%s\n", buffer); break;
      
    case LECTURA_ARCHIVO_PARAMETROS_INVALIDOS:
      printf("El archivo no existe o está dañado, revísalo e intenta más tarde.\n"); return -1;
    
    case LECTURA_ARCHIVO_ERROR:
      printf("Ocurrió un error leyendo el archivo, inténtalo más tarde.\n"); return -1;
      
    case LECTURA_ARCHIVO_MUY_GRANDE:
      printf("El archivo es demasiado grande, intenta con un archivo más pequeño.\n"); return -1;
      
    case LECTURA_ARCHIVO_SIN_MEMORIA:
      printf("El programa no cuenta con memoria suficiente para leer el archivo, intenta más tarde.\n"); return -1;
    }
  }else{ // Si el archivo no existe, inicializamos el búffer en un tamaño predefinido.
    buffer = malloc(sizeof(char) * TAMANIO_INICIAL);
    tamanio_buffer = TAMANIO_INICIAL;
    buffer[0] = EOF;
  }

  // Inicialización de nuestro ambiente de texto gráfico.
  setlocale(LC_ALL, "es_MX");                        // Colocamos la localización en español México.
  initscr();                                         // Iniciamos el modo gráfico.
  cbreak();                                          // Queremos capturar _todas_ las teclas que presione el usuario (esto incluye Control + C ¡aguas!)
  keypad(stdscr, TRUE);                              // Activamos el modo "keypad" para poder utilizar flechas del teclado y demás monerías.
  noecho();                                          // No queremos que las teclas que presione el usuario se impriman.
  getmaxyx(stdscr, filas_maximas, columnas_maximas); // Obtenemos las dimensiones de nuestra ventana.

  // Ejecutamos el editor principal.
  editor_de_texto(nombre_archivo, &buffer, &tamanio_buffer, filas_maximas, columnas_maximas);

  // Finalización del programa.
  clear();  // Limpiar la ventana.
  endwin(); // Finalizamos la ventana.
  free(buffer);
  return 0;
}

int leer_archivo_en_buffer(char *nombre_archivo, char **buffer_final, size_t *tamanio){
  char *buffer = NULL,
       *temporal;
  size_t tam = 0,
         usado = 0,
         n;
  FILE *archivo;

  archivo = fopen(nombre_archivo, "rb");

  // Validación de parámetros
  if (archivo == NULL || buffer_final == NULL || tamanio == NULL){ // Parámetros proporcionados son inválido.
    return LECTURA_ARCHIVO_PARAMETROS_INVALIDOS;
  }

  // Verificamos que no haya errores de lectura previos.
  if (ferror(archivo)){
    return LECTURA_ARCHIVO_ERROR;
  }

  // Ciclo principal de lectura.
  while (1){
    // Verificamos si llegamos al límite de lectura por "bloque".
    if ((usado + LECTURA_ARCHIVO_TAM_BLOQUE + 1) > tam){
      // Ajustamos el tamaño apropiadamente (siempre considerando EOF)
      tam = usado + LECTURA_ARCHIVO_TAM_BLOQUE + 1;

      // Checamos que no se desborden las variables, existen compiladores que optimizan esto automágicamente.
      if (tam <= usado){
        free(buffer);
        // Regresamos el error de que el archivo es más grande de lo que disponemos de memoria.
        return LECTURA_ARCHIVO_MUY_GRANDE;
      }

      temporal = realloc(buffer, tam); // Movemos la memoria desde nuestro buffer a la variable temporal.

      // Verificamos que aún tengamos memoria de "heap" disponible.
      if (temporal == NULL){
        free(buffer); // Regresamos el error de que nos quedamos sin memoria de "heap" suficiente.
        return LECTURA_ARCHIVO_SIN_MEMORIA;
      }
      buffer = temporal; // Asiganmos de vuelta el búffer.
    }

    // Usamos la variable temporal para medir la cantidad de datos que fueron leídos por la función.
    n = fread(buffer + usado, 1, LECTURA_ARCHIVO_TAM_BLOQUE, archivo);

    if (n == 0){// Verificamos si terminamos de leer el archivo.
      break;
    }
    usado += n; // Aumentamos el contador de memoria utilizada.
  }

  if (ferror(archivo)){// Verificamos que no haya existido un error de lectura.
    free(buffer);
    return LECTURA_ARCHIVO_ERROR; // Regresamos el error.
  }

  buffer = temporal; // Asignamos los datos leídos desde el buffer temporal.

  temporal = realloc(buffer, tam); // Movemos la memoria desde nuestro buffer a la variable temporal.

  if (temporal == NULL){ // Verificamos que aún tengamos memoria de "heap" disponible.
    free(buffer);
    return LECTURA_ARCHIVO_SIN_MEMORIA; // Regresamos el error de que nos quedamos sin memoria de "heap" suficiente.
  }

  buffer = temporal;// Asiganmos de vuelta el búffer.
  buffer[tam] = '\0'; // Asignamos el EOF.
  fclose(archivo);

  // Asignamos los datos leídos a los apuntadores de resultados.
  *buffer_final = buffer;
  *tamanio = usado;

  return LECTURA_ARCHIVO_OK; // Regresamos la bandera de éxito.
}

int existe_archivo(char *nombre_archivo){
  struct stat buffer; // Búffer para almacenar el resultado de stat().
  return (stat(nombre_archivo, &buffer) == 0); // Regresamos el valor de stat().
}

void editor_de_texto(char *nombre_archivo, char **buffer, size_t *tamanio_buffer, int filas_maximas, int columnas_maximas){
  int tecla_presionada, pos_x, pos_y, pos_buffer, fila_actual, **pantalla;

  pantalla = (int **)malloc((filas_maximas - 1) * sizeof(int *)); // Creamos "filas_maximas - 1" filas (la primer fila es para estadísticas) en nuestra pantalla.

  // Iteramos sobre cada fila de la pantalla.
  for (fila_actual = 0; fila_actual < (filas_maximas - 1); fila_actual++){
    // Generamos el arreglo de "columnas_maximas" columnas para cada fila.
    pantalla[fila_actual] = (int *)malloc(columnas_maximas * sizeof(int));
  }

  // Colocamos la posición inicial del cursor y de la posición en el búffer.
  pos_x = 0;
  pos_y = 0;
  pos_buffer = 0;

  do{ // Ciclo principal de ejecución.
    // Calculamos la pantalla actual a partir de la posición del búffer y coordenadas de cursor actuales.
    calcular_pantalla(pantalla, filas_maximas, columnas_maximas, pos_y, pos_buffer, buffer, *tamanio_buffer);

    mostrar_pantalla(pos_x, pos_y, buffer, filas_maximas, columnas_maximas, pantalla); // Mostrar el contenido del archivo corresopndiente a la ventana.

    mostrar_estadisticas(nombre_archivo, buffer, *tamanio_buffer, pos_x, pos_y, pos_buffer, columnas_maximas); // Mostramos las coordenadas y posición en el búffer

    tecla_presionada = getch();// Leemos la tecla que presiona el usuario.

    switch (tecla_presionada){ // Si la tecla no es ESC, editamos el búffer apropiadamente.
    case 27:
      tecla_esc(nombre_archivo, buffer, *tamanio_buffer); break;

    case KEY_UP:
      tecla_arriba(columnas_maximas, pantalla, &pos_y, &pos_x, buffer, &pos_buffer); break;

    case KEY_DOWN:
      tecla_abajo(filas_maximas, pantalla, &pos_y, &pos_x, buffer, &pos_buffer); break;

    case KEY_LEFT:
      tecla_izquierda(&pos_x, &pos_buffer); break;

    case KEY_RIGHT:
      tecla_derecha(&pos_x, &pos_buffer, columnas_maximas, (*tamanio_buffer), buffer); break;

    case 127:
      tecla_borrar(&pos_x, &pos_y, buffer, tamanio_buffer, &pos_buffer, pantalla); break;

    default:
      tecla_cualquiera(buffer, tamanio_buffer, tecla_presionada, &pos_buffer, &pos_x, columnas_maximas, &pos_y, filas_maximas);
      break;
    }
  } while (tecla_presionada != 27); // Mientras el usuario no presiones ESC.
  free(pantalla);
}

void calcular_pantalla(int **pantalla, int filas_maximas, int columnas_maximas, int pos_y, int pos_buffer, char **buffer, size_t tamanio_buffer){
  int pos_buffer_temporal,
      pos_x_temporal,
      pos_y_temporal,
      lineas_antes;

  // Llenamos la pantalla de -1's.
  for (pos_y_temporal = 0; pos_y_temporal < (filas_maximas - 1); pos_y_temporal++){
    for (pos_x_temporal = 0; pos_x_temporal < columnas_maximas; pos_x_temporal++){
      pantalla[pos_y_temporal][pos_x_temporal] = -1;
    }
  }

  // Calculamos el número de líneas previas a la posición actual en el búffer mientras reducimos dicha posición.
  for (pos_buffer_temporal = pos_buffer - 1, lineas_antes = 0; pos_buffer_temporal > 0 && lineas_antes < pos_y; pos_buffer_temporal--){
    if ((*buffer)[pos_buffer_temporal] == '\n'){
      lineas_antes += 1;
    }
  }

  if (pos_buffer_temporal == -1){
    pos_buffer_temporal = 0;
  }

  // Reducimos la posición del búffer para encontrar el principio de la línea inicial.
  while (pos_buffer_temporal > 0 && (*buffer)[pos_buffer_temporal - 1] != '\n'){
    pos_buffer_temporal--;
  }

  // Iteramos sobre cada fila de la pantalla hasta encontrar el fin del búffer.
  for (pos_y_temporal = 0; pos_y_temporal < (filas_maximas - 1) && pos_buffer_temporal < ((int)(tamanio_buffer)) && (*buffer)[pos_buffer_temporal] != '\0'; pos_y_temporal++, pos_buffer_temporal++){
    // Iteramos sobre cada columna y caracter del búffer.
    for (pos_x_temporal = 0; pos_x_temporal < columnas_maximas && pos_buffer_temporal < ((int)(tamanio_buffer)) && (*buffer)[pos_buffer_temporal] != '\n' && (*buffer)[pos_buffer_temporal] != '\0'; pos_x_temporal++, pos_buffer_temporal++){
      // Asignamos el valor presente en el búffer.
      pantalla[pos_y_temporal][pos_x_temporal] = pos_buffer_temporal;
    }

    // Verificamos si nuestro último caracter fue un salto de línea o EOL y en caso contrario sólamente recorremos hasta encontrarlo.
    if ((*buffer)[pos_buffer_temporal] != '\n' && (*buffer)[pos_buffer_temporal] != '\0'){
      while (pos_buffer_temporal < ((int)(tamanio_buffer)) && (*buffer)[pos_buffer_temporal] != '\n' && (*buffer)[pos_buffer_temporal] != '\0'){
        pos_buffer_temporal++;
      }
    }
    pantalla[pos_y_temporal][pos_x_temporal] = pos_buffer_temporal; // Asignamos el último caracter encontrado a nuestra posición.
  }
}

void tecla_abajo(int filas_maximas, int **pantalla, int *pos_y, int *pos_x, char **buffer, int *pos_buffer)
{
  int pos_x_temporal,
      pos_y_temporal,
      pos_buffer_temporal;

  // Primero verificamos si aún hay filas presentes en la pantalla por debajo de la posición actual.
  if ((*pos_y) < (filas_maximas - 2)){
    // Aumentamos nuestra posición en y.
    pos_y_temporal = (*pos_y) + 1;
    pos_x_temporal = (*pos_x);

    // Verificamos que no sea un espacio vacío.
    if (pantalla[pos_y_temporal][pos_x_temporal] == -1){
      // Si es un espacio vacío (en x) reducimos la coordenada hasta encontrar un texto.
      while (pantalla[pos_y_temporal][pos_x_temporal] == -1 && pos_x_temporal){
        pos_x_temporal--;
      }

      // Si la coordenada en la posición encontrada contiene texto, asignamos la posición en el búffer
      // en otro caso descartamos la operación.
      if (pantalla[pos_y_temporal][pos_x_temporal] != -1){
        (*pos_buffer) = pantalla[pos_y_temporal][pos_x_temporal];
        (*pos_y) = pos_y_temporal;
        (*pos_x) = pos_x_temporal;
      }
    }else{ // Si no es un espacio vacío, asignamos directamente.
      (*pos_buffer) = pantalla[pos_y_temporal][pos_x_temporal];
      (*pos_y) = pos_y_temporal;
      (*pos_x) = pos_x_temporal;
    }
  }else{
    // En caso de que no haya filas presentes en la pantalla por debajo de la posición actual, debemos verificar que contemos, entonces,
    // con una siguiente línea en el búffer.
    for (pos_buffer_temporal = (*pos_buffer); (*buffer)[pos_buffer_temporal] != '\n' && (*buffer)[pos_buffer_temporal] != '\0'; pos_buffer_temporal++)
      ;

    // Si el caracter encontrado NO es un EOL (es un \n), entonces procedemos con otras verificaciones.
    if ((*buffer)[pos_buffer_temporal] == '\n'){
      pos_buffer_temporal++; // Aumentamos la posición del búffer temporal a la siguiente línea.

      // Lo primero será calcular la posición de x en la línea más cercana a la posición actual.
      for (pos_x_temporal = 0; pos_x_temporal < (*pos_x) && (*buffer)[pos_buffer_temporal + pos_x_temporal] != '\n' && (*buffer)[pos_buffer_temporal + pos_x_temporal] != '\0'; pos_x_temporal++)
        ;

      // Asignamos la posición en x encontrada a nuestra variable.
      (*pos_x) = pos_x_temporal;
      (*pos_buffer) = pos_buffer_temporal;
    }
  }
}

void tecla_arriba(int columnas_maximas, int **pantalla, int *pos_y, int *pos_x, char **buffer, int *pos_buffer){
  int pos_y_temporal = (*pos_y),
      pos_x_temporal = (*pos_x),
      pos_buffer_temporal = (*pos_buffer);

  // Verificamos si aún disponemos de líneas disponibles en pantalla.
  if (pos_y_temporal){
    pos_y_temporal--; // Reducimos nuestra posición en y.

    // Verificamos que no sea un espacio vacío.
    if (pantalla[pos_y_temporal][pos_x_temporal] == -1){
      // Si es un espacio vacío (en x) reducimos la coordenada hasta encontrar un texto.
      while (pantalla[pos_y_temporal][pos_x_temporal] == -1 && pos_x_temporal){
        pos_x_temporal--;
      }
    }
    (*pos_buffer) = pantalla[pos_y_temporal][pos_x_temporal]; // Asignamos la posición del búffer a la indicada por la pantalla.

    // Asignamos las variables calculadas.
    (*pos_y) = pos_y_temporal;
    (*pos_x) = pos_x_temporal;
  }else{
    if (pos_buffer_temporal > 0 && (*buffer)[pos_buffer_temporal] == '\n'){
      --pos_buffer_temporal;
    }

    // En caso de que no dispongamos de líneas disponibles arriba en la pantalla, verificamos si contamos con líneas previas en el búffer.
    while (pos_buffer_temporal > 0 && (*buffer)[pos_buffer_temporal] != '\n'){
      pos_buffer_temporal--;
    }

    // En caso de que si contemos con líneas previas, cargamos la línea inmediata anterior.
    if (pos_buffer_temporal > 0 && (*buffer)[pos_buffer_temporal] == '\n'){
      for (pos_x_temporal = 0; pos_x_temporal < columnas_maximas && (pos_buffer_temporal - pos_x_temporal) > 0 && (*buffer)[pos_buffer_temporal - pos_x_temporal] != '\n'; pos_x_temporal++)
        ;

      (*pos_buffer) = pos_buffer_temporal;
      (*pos_x) = pos_x_temporal;
    }
  }
}

void tecla_izquierda(int *pos_x, int *pos_buffer){
  // En caso de que sea posible, reducimos la posición en x del cursor.
  if ((*pos_x) && (*pos_buffer)){
    (*pos_x)--;
    (*pos_buffer)--;
  }
}

void tecla_derecha(int *pos_x, int *pos_buffer, int columnas_maximas, size_t tamanio_buffer, char **buffer){
  // En caso de que sea posible, aumentamos la posición en x del cursor.
  if ((*pos_x) < columnas_maximas && (*pos_buffer) < ((int)tamanio_buffer) && (*buffer)[(*pos_buffer)] != '\n' && (*buffer)[(*pos_buffer)] != '\0'){
    (*pos_x)++;
    (*pos_buffer)++;
  }
}

void tecla_borrar(int *pos_x, int *pos_y, char **buffer, size_t *tamanio_buffer, int *pos_buffer, int **pantalla){
  // Verificamos que estemos en una posición válida para eliminar el caracter.
  if ((*pos_buffer) - 1 >= 0){
    // Reducimos el tamaño de la memoria y recorremos la memoria restante en nuestro búffer un caracter.
    memmove(&(*buffer)[(*pos_buffer) - 1], &(*buffer)[(*pos_buffer)], ((int)(--(*tamanio_buffer))) * sizeof(*(*buffer)));

    (*pos_buffer)--;// Reducimos nuestra posición en el búffer.

    // Verificamos si podemos reducir nuestra posición en x actual en la pantalla (columnas)
    if ((*pos_x)){
      (*pos_x)--;
    }else if ((*pos_y)){
      // En caso de que no podamos reducir nuestra posición en x (columnas) pero si en y (filas), buscamos el último caracter
      // de la siguiente línea en el búffer.
      while (pantalla[(*pos_y) - 1][(*pos_x) + 1] != -1){
        (*pos_x)++;
      }
      (*pos_y)--;
    }
  }
}

void tecla_cualquiera(char **buffer, size_t *tamanio_buffer, int tecla_presionada, int *pos_buffer, int *pos_x, int columnas_maximas, int *pos_y, int filas_maximas){
  int pos_buffer_temporal; // Para iterar dentro de nuestro búffer de memoria.
  char *buffer_temporal;   // Para realizar la copia del búffer.

  // Creamos un nuevo búffer con el tamaño aumentado.
  buffer_temporal = malloc(((*tamanio_buffer) + 1) * sizeof(char));

  for (pos_buffer_temporal = 0; pos_buffer_temporal < (*pos_buffer); pos_buffer_temporal++){
    buffer_temporal[pos_buffer_temporal] = (*buffer)[pos_buffer_temporal];
  }

  // Recorremos los elementos dentro de nuestro búffer de memoria en una unidad.
  for (pos_buffer_temporal = ((int)(*tamanio_buffer)); pos_buffer_temporal > (*pos_buffer); pos_buffer_temporal--){
    buffer_temporal[pos_buffer_temporal] = (*buffer)[pos_buffer_temporal - 1];
  }

  // Si la tecla es "Enter", manualmente colocamos el caracter y aumentamos nuestra posición en y.
  if (tecla_presionada == '\n'){
    buffer_temporal[pos_buffer_temporal] = '\n'; // Asignamos el salto de línea al búffer.

    // Verificamos si podemos aumentar la coordenada en y.
    if ((*pos_y) < (filas_maximas - 1)){
      (*pos_y)++;
      (*pos_x) = 0;
    }
  }else{
    // Si es cualquier otra tecla, hacemos un casting a caracter con su código ASCII.
    buffer_temporal[pos_buffer_temporal] = (char)tecla_presionada;
  }
  (*tamanio_buffer) += 1; // Aumentamos el tamaño del búffer.
  free((*buffer)); // Borramos el búffer anterior.
  (*buffer) = buffer_temporal; // Asignamos el nuevo búffer.
  (*pos_buffer) += 1; // Aumentamos la posición dentro del búffer.

  // Si es posible, aumentamos la posición en x (columna) en nuestra pantalla actualmente.
  if ((*pos_x + 1) < columnas_maximas){
    (*pos_x)++;
  }
}

void tecla_esc(char *nombre_archivo, char **buffer, size_t tamanio_buffer){
  FILE *archivo; // El archivo a abrir para guardar los datos.

  archivo = fopen(nombre_archivo, "w");

  // Verificamos que se haya podido abrir el archivo.
  if (archivo == NULL){
    exit(-1);
    return;
  }

  // Verificamos que el búffer esté terminado con un EOF, sino, lo añadimos.
  if ((*buffer)[tamanio_buffer] != EOF){
    (*buffer)[tamanio_buffer] = EOF;
  }
  fwrite(*buffer, sizeof(char), tamanio_buffer, archivo); // Guardamos nuestro búffer en el archivo.
  fclose(archivo);
}

void mostrar_pantalla(int pos_x, int pos_y, char **buffer, int filas_maximas, int columnas_maximas, int **pantalla){
  char caracter_temporal;
  int fila_actual, columna_actual;  

  // Iteramos en las filas de la matriz.
  for (fila_actual = 0; fila_actual < (filas_maximas - 1); fila_actual++){
    // Iteramos sobre las columnas de la matriz.
    for (columna_actual = 0; columna_actual < columnas_maximas; columna_actual++){
      // Si es la posición de nuestro cursor, activamos el modo invertido para imprimir el texto.
      if (fila_actual == pos_y && columna_actual == pos_x){
        attron(A_REVERSE);
      }

      // Seleccionamos el tipo de caracter a imprimir.
      if (pantalla[fila_actual][columna_actual] == -1){
        caracter_temporal = CARACTER_VACIO; // En caso de estar vacío, imprimimos un caracter vacío.
      }else if ((*buffer)[pantalla[fila_actual][columna_actual]] == '\n' || (*buffer)[pantalla[fila_actual][columna_actual]] == '\0'){
        caracter_temporal = CARACTER_DE_CONTROL; // En caso de ser EOL o un salto de línea, usamos un caracter de control.
      }else{
        // En cualquier otro caso, mostramos el caracter presente en el índice almacenado en pantalla.
        caracter_temporal = (*buffer)[pantalla[fila_actual][columna_actual]];
      }
      mvprintw(fila_actual + 1, columna_actual, "%c", caracter_temporal); // Imprimimos el caracter correspondiente en la coordenada actual.

      // Si es la posición de nuestro cursor, desactivamos el modo invertido para imprimir el texto.
      if (fila_actual == pos_y && columna_actual == pos_x){
        attroff(A_REVERSE);
      }
    }
  }
}

void mostrar_estadisticas(char *nombre_archivo, char **buffer, size_t tamanio_buffer, int pos_x, int pos_y, int pos_buffer, int columnas_maximas){
  int lineas, linea_actual, pos_buffer_temporal;

  // Ciclo para contar el número de líneas presentes en nuestro editor actual.
  for (pos_buffer_temporal = 0, lineas = 0, linea_actual = 0; pos_buffer_temporal < ((int)tamanio_buffer); pos_buffer_temporal++){
    // Para saber la línea actual, contabilizamos únicamente aquellas antes de nuestra posición actual en el búffer.
    if (pos_buffer_temporal < pos_buffer){
      linea_actual += (*buffer)[pos_buffer_temporal] == '\n';
    }
    lineas += (*buffer)[pos_buffer_temporal] == '\n'; // Contabilización de líneas generales.
  }

  // Mostramos el "menú" de nuestro editor.
  attron(A_REVERSE);
  mvhline(0, 0, '#', columnas_maximas);
  mvprintw(0, 0, "Editando \"%s\" %s | ESC para Guardar y Salir | Líneas: %i | Línea Actual: %i | Col: %i | Fila: %i | Pos. Búffer: %i", nombre_archivo, existe_archivo(nombre_archivo) ? "" : "(Nuevo)", lineas, linea_actual, pos_x, pos_y, pos_buffer);
  attroff(A_REVERSE);
}
