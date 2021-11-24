#include <stdio.h>  
#include <stdlib.h>   
#include <sys/stat.h> 
#include <ncurses.h>  
#include <string.h>   
#include <locale.h>   

#define BLOCK_SIZE 2097152 // Tamaño de lectura por bloque de info aprox. 2 MB.  
#define CARACTER_VACIO ' '                 // Cuando la pantalla no debe mostrar nada
#define CARACTER_DE_CONTROL ' '            // Cuando existe un caracter de control (\0 o \n), para debuggear más fácil qué está sucediendo.

// Prototipos de funciones.
int saveFileInBuffer(char *, char **, size_t *);                        // Lee fileFichero y guarda en buffer.
int checkFileExists(char *);                                            // Verifica si fileFichero existe.
void textEditor(char *, char **, size_t *, int, int);                   // Realiza operaciones del editor.
void calcContentsScreen(int **, int, int, int, int, char **, size_t);   // Analiza los contenidos de la pantalla en cada interaccion.
void showScreen(int, int, char **, int, int, int **);                   // Muestra la pantalla
void showStatistics(char *, char **, size_t, int, int, int, int);       // Muestra informacion en la barra superior
void downKey(int, int **, int *, int *, char **, int *);                // Mueve cursor hacia abajo
void upKey(int, int **, int *, int *, char **, int *);                  // Mueve cursor hacia arriba
void leftKey(int *, int *);                                             // Mueve cursor hacia la izquierda
void rightKey(int *, int *, int, size_t, char **);                      // Mueve cursor hacia la derecha
void delKey(int *, int *, char **, size_t *, int *, int **);            // Realiza borrado de texto al presionar tecla Retroceso
void escKey(char *, char **, size_t);                                   // Salir de programa con tecla Esc
void anyKey(char **, size_t *, int, int *, int *, int, int *, int);     // Inserta cualquier caracter disponible en teclado

int main(int argc, char *argv[]){
  char *buffer, *fileName;         
  int readFileResult, maxRows, maxCols, fileExist;      
  size_t bufferSize;        

  // Verifica si se Ingreso nombre del fileFichero como parametro.
  if (argc == 2){ // Se ingreso nombre de fileFichero como parametro
    fileName = argv[1];
  }else{ // No se ingreso nombre de fileFichero como parametro
    printf("Para editar un fileFichero, ingrese el siguiente comando:\n");
    printf("%s /ruta_completa/fileFichero.txt\n\no intente con ruta relativa:\n\n", argv[0]);
    printf("%s ./fileFichero.txt\n\n", argv[0]);
    return 0;
  }

  fileExist = checkFileExists(fileName); // Verifica que fileFichero exista.

  if (fileExist){ // Se lee contenido de fileFichero si este existe
    printf("Abriendo '%s' ...\n", fileName);

    readFileResult = saveFileInBuffer(fileName, &buffer, &bufferSize); // Lee fileFichero y guarda en buffer

    // Elije posibles casos de lectura de fileFichero
    switch (readFileResult){
    case 0: // Lectura Correcta
      printf("¡Archivo leido con exito!\n%s\n", buffer); break;
      
    case -1: // Parametros Invalidos
      printf("El Archivo no existe o esta dañado.\n"); return -1;
    
    case -2: // Error de Lectura
      printf("Ocurrio un error de lectura, intente de nuevo.\n"); return -1;
      
    case -3: // fileFichero muy grande
      printf("Archivo demasiado grande, intente con otro fileFichero.\n"); return -1;
      
    case -4: // Memoria insuficiente
      printf("Programa con memoria insuficiente para leer el fileFichero, intente de nuevo.\n"); return -1;
    }
  }else{ // fileFichero no existe
    buffer = malloc(sizeof(char) * 2); // Se inicia el buffer en un tamaño predefinido.
    bufferSize = 2;
    buffer[0] = EOF;
  }

  // Inicializacion de ambiente grafico.
  setlocale(LC_ALL, "es_MX");                       
  initscr();                                        
  cbreak();                                          
  keypad(stdscr, TRUE);                              
  noecho();                                          
  getmaxyx(stdscr, maxRows, maxCols); // Obtiene dimensiones de ventana.
  textEditor(fileName, &buffer, &bufferSize, maxRows, maxCols); // Se ejecuta el editor de texto.

  // Finaliza programa.
  clear();  
  endwin(); 
  free(buffer);
  return 0;
}

/*######################################################### FUNCIONES ############################################################################################*/

int saveFileInBuffer(char *fileName, char **finalBuffer, size_t *auxSize){
  char *buffer = NULL,
       *temp;
  size_t tam = 0,
         usado = 0,
         n;
  FILE *fileFichero;

  fileFichero = fopen(fileName, "rb");

  if (fileFichero == NULL || finalBuffer == NULL || auxSize == NULL){ // Valida parametros
    return -1; // Parametros invalidos.
  }

  if (ferror(fileFichero)){ // Verifica inexistencia de errores de lectura.
    return -2; // Error de Lectura
  }

  while (1){ // Bucle de lectura.
    if ((usado + BLOCK_SIZE + 1) > tam){ // Verifica si se llego al límite de lectura por bloque.
      // Se ajusta tamaño considerando EOF
      tam = usado + BLOCK_SIZE + 1;

      if (tam <= usado){// Verifica que variables no se desborden
        free(buffer);
        return -3; // Archivo muy grande
      }
      temp = realloc(buffer, tam); // Mueve memoria de buffer a temp.

      // Verifica que aun exista memoria disponible.
      if (temp == NULL){
        free(buffer);
        return -4; // Memoria Insuficiente
      }
      buffer = temp; // Se vuelve a asignar memoria de temp a buffer.
    }

    // Se mide la cantidad de datos leídos por la funcion.
    n = fread(buffer + usado, 1, BLOCK_SIZE, fileFichero);

    if (n == 0){// Verifica si finalizo lectura de archivo.
      break;
    }
    usado += n; // Aumenta contador de memoria utilizada.
  }

  if (ferror(fileFichero)){// Verifica inexistencia de error de lectura.
    free(buffer);
    return -2; // Error de Lectura.
  }
  buffer = temp;
  temp = realloc(buffer, tam);

  if (temp == NULL){ // Verifica que exista memoria disponible.
    free(buffer);
    return -4; // Memoria Insuficiente
  }
  buffer = temp;
  buffer[tam] = '\0'; // Asigna EOF.
  fclose(fileFichero);

  // Asigna datos leídos a los apuntadores de resultados.
  *finalBuffer = buffer;
  *auxSize = usado;
  return 0; // Lectura Exitosa
}

int checkFileExists(char *fileName){
  struct stat buffer; // Almacena resultado de funcion stat()
  return (stat(fileName, &buffer) == 0);
}

void textEditor(char *fileName, char **buffer, size_t *bufferSize, int maxRows, int maxCols){
  int pressedKey, pos_x, pos_y, pos_buffer, actualRow, **pantalla;

  pantalla = (int **)malloc((maxRows - 1) * sizeof(int *)); // Se crean n-1 filas en pantalla (reservado para barra)

  for (actualRow = 0; actualRow < (maxRows - 1); actualRow++){ // Recorre cada fila de pantalla
    pantalla[actualRow] = (int *)malloc(maxCols * sizeof(int)); // Genera columnas para cada fila
  }

  // Inicializa posicion de cursor y buffer
  pos_x = 0;
  pos_y = 0;
  pos_buffer = 0;

  do{ // Bucle de ejecucion.
    calcContentsScreen(pantalla, maxRows, maxCols, pos_y, pos_buffer, buffer, *bufferSize); // Calcula contenido de pantalla a partir de posicion de buffer y cursor actuales.
    showScreen(pos_x, pos_y, buffer, maxRows, maxCols, pantalla); // Muestra contenido del archivo en pantalla.
    showStatistics(fileName, buffer, *bufferSize, pos_x, pos_y, pos_buffer, maxCols); // Muestra coordenadas y posicion de buffer
    pressedKey = getch();// Lee tecla presionada por usuario.

    switch (pressedKey){
    case 27: // Presiona Esc
      escKey(fileName, buffer, *bufferSize); break;

    case KEY_UP:
      upKey(maxCols, pantalla, &pos_y, &pos_x, buffer, &pos_buffer); break;

    case KEY_DOWN:
      downKey(maxRows, pantalla, &pos_y, &pos_x, buffer, &pos_buffer); break;

    case KEY_LEFT:
      leftKey(&pos_x, &pos_buffer); break;

    case KEY_RIGHT:
      rightKey(&pos_x, &pos_buffer, maxCols, (*bufferSize), buffer); break;

    case 127: // Presiona Backspace
      delKey(&pos_x, &pos_y, buffer, bufferSize, &pos_buffer, pantalla); break;

    default:
      anyKey(buffer, bufferSize, pressedKey, &pos_buffer, &pos_x, maxCols, &pos_y, maxRows);
      break;
    }
  } while (pressedKey != 27);
  free(pantalla);
}

void calcContentsScreen(int **pantalla, int maxRows, int maxCols, int pos_y, int pos_buffer, char **buffer, size_t bufferSize){
  int pos_buffer_temp,
      pos_x_temp,
      pos_y_temp,
      oldLines;

  for (pos_y_temp = 0; pos_y_temp < (maxRows - 1); pos_y_temp++){
    for (pos_x_temp = 0; pos_x_temp < maxCols; pos_x_temp++){
      pantalla[pos_y_temp][pos_x_temp] = -1;
    }
  }

  // Calcula numero de líneas previas a la posicion actual en el buffer mientras reduce esa posicion.
  for (pos_buffer_temp = pos_buffer - 1, oldLines = 0; pos_buffer_temp > 0 && oldLines < pos_y; pos_buffer_temp--){
    if ((*buffer)[pos_buffer_temp] == '\n'){
      oldLines += 1;
    }
  }

  if (pos_buffer_temp == -1){
    pos_buffer_temp = 0;
  }

  // Reduce posicion de buffer para encontrar el principio de la linea inicial.
  while (pos_buffer_temp > 0 && (*buffer)[pos_buffer_temp - 1] != '\n'){
    pos_buffer_temp--;
  }

  // Recorre cada fila de la pantalla hasta encontrar el fin del buffer.
  for (pos_y_temp = 0; pos_y_temp < (maxRows - 1) && pos_buffer_temp < ((int)(bufferSize)) && (*buffer)[pos_buffer_temp] != '\0'; pos_y_temp++, pos_buffer_temp++){
    // Recorre cada columna y caracter del buffer.
    for (pos_x_temp = 0; pos_x_temp < maxCols && pos_buffer_temp < ((int)(bufferSize)) && (*buffer)[pos_buffer_temp] != '\n' && (*buffer)[pos_buffer_temp] != '\0'; pos_x_temp++, pos_buffer_temp++){
      // Asigna valor presente en el buffer.
      pantalla[pos_y_temp][pos_x_temp] = pos_buffer_temp;
    }

    // Verifica si ultimo caracter ingresado fue salto de línea o EOL
    if ((*buffer)[pos_buffer_temp] != '\n' && (*buffer)[pos_buffer_temp] != '\0'){
      while (pos_buffer_temp < ((int)(bufferSize)) && (*buffer)[pos_buffer_temp] != '\n' && (*buffer)[pos_buffer_temp] != '\0'){
        pos_buffer_temp++;
      }
    }
    pantalla[pos_y_temp][pos_x_temp] = pos_buffer_temp; // Asigna ultimo caracter ingresado en posicion actual
  }
}

void downKey(int maxRows, int **pantalla, int *pos_y, int *pos_x, char **buffer, int *pos_buffer)
{
  int pos_x_temp,
      pos_y_temp,
      pos_buffer_temp;

  // Verifica si aUn hay filas en la pantalla por debajo de posición actual.
  if ((*pos_y) < (maxRows - 2)){
    // Aumenta posicion y
    pos_y_temp = (*pos_y) + 1;
    pos_x_temp = (*pos_x);

    // Verifica que no sea un espacio vacio.
    if (pantalla[pos_y_temp][pos_x_temp] == -1){
      // Si es un espacio vacio en x, se reduce la coordenada hasta encontrar texto.
      while (pantalla[pos_y_temp][pos_x_temp] == -1 && pos_x_temp){
        pos_x_temp--;
      }

      // Si coordenada contiene texto, se asigna la posicion en buffer
      if (pantalla[pos_y_temp][pos_x_temp] != -1){
        (*pos_buffer) = pantalla[pos_y_temp][pos_x_temp];
        (*pos_y) = pos_y_temp;
        (*pos_x) = pos_x_temp;
      }
    }else{ // Si no es un espacio vacio, asigna directamente.
      (*pos_buffer) = pantalla[pos_y_temp][pos_x_temp];
      (*pos_y) = pos_y_temp;
      (*pos_x) = pos_x_temp;
    }
  }else{ // No haya filas presentes en la pantalla por debajo de posición actual
    // Verifica que se tenga una siguiente linea en buffer.
    for (pos_buffer_temp = (*pos_buffer); (*buffer)[pos_buffer_temp] != '\n' && (*buffer)[pos_buffer_temp] != '\0'; pos_buffer_temp++)
      ;

    // Verifica que caracter encontrado NO es un EOL (\n)
    if ((*buffer)[pos_buffer_temp] == '\n'){
      pos_buffer_temp++; // Aumenta posicion de buffer temp a la siguiente linea.

      // Calcula posicion de x en linea mas cercana a posicion actual.
      for (pos_x_temp = 0; pos_x_temp < (*pos_x) && (*buffer)[pos_buffer_temp + pos_x_temp] != '\n' && (*buffer)[pos_buffer_temp + pos_x_temp] != '\0'; pos_x_temp++)
        ;

      // Asigna la posicion en x encontrada a variable.
      (*pos_x) = pos_x_temp;
      (*pos_buffer) = pos_buffer_temp;
    }
  }
}

void upKey(int maxCols, int **pantalla, int *pos_y, int *pos_x, char **buffer, int *pos_buffer){
  int pos_y_temp = (*pos_y),
      pos_x_temp = (*pos_x),
      pos_buffer_temp = (*pos_buffer);

  // Verifica si aun hay lineas disponibles en pantalla.
  if (pos_y_temp){
    pos_y_temp--; // Reduce posicion en y.

    // Verifica que no sea un espacio vacio.
    if (pantalla[pos_y_temp][pos_x_temp] == -1){
      // Si es espacio vacío en x, se reduce la coordenada hasta encontrar texto.
      while (pantalla[pos_y_temp][pos_x_temp] == -1 && pos_x_temp){
        pos_x_temp--;
      }
    }
    (*pos_buffer) = pantalla[pos_y_temp][pos_x_temp]; // Asigna posicion del buffer a la indicada por la pantalla.

    // Asigna las variables calculadas.
    (*pos_y) = pos_y_temp;
    (*pos_x) = pos_x_temp;
  }else{
    if (pos_buffer_temp > 0 && (*buffer)[pos_buffer_temp] == '\n'){
      --pos_buffer_temp;
    }

    // En caso de que no contar con lineas disponibles arriba en pantalla, se verifica si existen lineas previas en buffer.
    while (pos_buffer_temp > 0 && (*buffer)[pos_buffer_temp] != '\n'){
      pos_buffer_temp--;
    }

    // Se carga la línea inmediata anterior si se cuenta con lineas previas en buffer.
    if (pos_buffer_temp > 0 && (*buffer)[pos_buffer_temp] == '\n'){
      for (pos_x_temp = 0; pos_x_temp < maxCols && (pos_buffer_temp - pos_x_temp) > 0 && (*buffer)[pos_buffer_temp - pos_x_temp] != '\n'; pos_x_temp++)
        ;

      (*pos_buffer) = pos_buffer_temp;
      (*pos_x) = pos_x_temp;
    }
  }
}

void leftKey(int *pos_x, int *pos_buffer){
  // Reduce posicion en x del cursor
  if ((*pos_x) && (*pos_buffer)){
    (*pos_x)--;
    (*pos_buffer)--;
  }
}

void rightKey(int *pos_x, int *pos_buffer, int maxCols, size_t bufferSize, char **buffer){
  // Aumenta posicion en x del cursor.
  if ((*pos_x) < maxCols && (*pos_buffer) < ((int)bufferSize) && (*buffer)[(*pos_buffer)] != '\n' && (*buffer)[(*pos_buffer)] != '\0'){
    (*pos_x)++;
    (*pos_buffer)++;
  }
}

void delKey(int *pos_x, int *pos_y, char **buffer, size_t *bufferSize, int *pos_buffer, int **pantalla){
  // Verifica que exista una posicion valida para eliminar el caracter.
  if ((*pos_buffer) - 1 >= 0){
    // Reduce el tamaño de la memoria y recorre la memoria restante en buffer un caracter.
    memmove(&(*buffer)[(*pos_buffer) - 1], &(*buffer)[(*pos_buffer)], ((int)(--(*bufferSize))) * sizeof(*(*buffer)));

    (*pos_buffer)--;// Reduce posicion en buffer.

    // Verifica si se puede reducir posicion en x actual en la pantalla (columnas)
    if ((*pos_x)-1 >=0){
      (*pos_x)--;
    }else if ((*pos_y)-1 >=0){
      // En caso de no poder reducir posición en x (columnas) pero si en y (filas), se busca el ultimo caracter de la siguiente linea en el buffer.
      while (pantalla[(*pos_y) - 1][(*pos_x) + 1] != -1){
        (*pos_x)++;
      }
      (*pos_y)--;
    }
  }
}

void anyKey(char **buffer, size_t *bufferSize, int pressedKey, int *pos_buffer, int *pos_x, int maxCols, int *pos_y, int maxRows){
  int pos_buffer_temp;
  char *buffer_temp;   

  // Crea un nuevo buffer con el tamaño aumentado.
  buffer_temp = malloc(((*bufferSize) + 1) * sizeof(char));

  for (pos_buffer_temp = 0; pos_buffer_temp < (*pos_buffer); pos_buffer_temp++){
    buffer_temp[pos_buffer_temp] = (*buffer)[pos_buffer_temp];
  }

  // Recorre los elementos dentro del buffer de memoria en una unidad.
  for (pos_buffer_temp = ((int)(*bufferSize)); pos_buffer_temp > (*pos_buffer); pos_buffer_temp--){
    buffer_temp[pos_buffer_temp] = (*buffer)[pos_buffer_temp - 1];
  }

  // Si la tecla es "Enter", manualmente coloca el caracter y aumenta posicion en y.
  if (pressedKey == '\n'){
    buffer_temp[pos_buffer_temp] = '\n'; // Asigna el salto de linea al buffer.

    // Verifica si podemos aumentar la coordenada en y.
    if ((*pos_y) < (maxRows - 1)){
      (*pos_y)++;
      (*pos_x) = 0;
    }
  }else{
    // Si es cualquier otra tecla, hace un casting a caracter con su codigo ASCII.
    buffer_temp[pos_buffer_temp] = (char)pressedKey;
  }
  (*bufferSize) += 1; // Aumenta el tamaño del buffer.
  free((*buffer)); // Borra el buffer anterior.
  (*buffer) = buffer_temp; // Asigna el nuevo buffer.
  (*pos_buffer) += 1; // Aumenta la posicion dentro del buffer.

  // Si es posible, aumenta la posicion en x (columna) en pantalla actual.
  if ((*pos_x + 1) < maxCols){
    (*pos_x)++;
  }
}

void escKey(char *fileName, char **buffer, size_t bufferSize){
  FILE *fileFichero; 

  fileFichero = fopen(fileName, "w");

  // Verifica que se haya podido abrir el archivo.
  if (fileFichero == NULL){
    exit(-1);
    return;
  }

  // Verifica que el buffer este terminado con un EOF, en caso contrario lo añade.
  if ((*buffer)[bufferSize] != EOF){
    (*buffer)[bufferSize] = EOF;
  }
  fwrite(*buffer, sizeof(char), bufferSize, fileFichero); // Guarda el buffer en archivo.
  fclose(fileFichero);
}

void showScreen(int pos_x, int pos_y, char **buffer, int maxRows, int maxCols, int **pantalla){
  char caracter_temp;
  int actualRow, columna_actual;  

  // Recorre las filas de la matriz.
  for (actualRow = 0; actualRow < (maxRows - 1); actualRow++){
    // Recorre las columnas de la matriz.
    for (columna_actual = 0; columna_actual < maxCols; columna_actual++){
      // Si es la posicion del cursor, activa el modo invertido para imprimir texto.
      if (actualRow == pos_y && columna_actual == pos_x){
        attron(A_REVERSE);
      }

      // Selecciona el tipo de caracter a imprimir.
      if (pantalla[actualRow][columna_actual] == -1){
        caracter_temp = CARACTER_VACIO; // En caso de estar vacio, imprime un caracter vacio.
      }else if ((*buffer)[pantalla[actualRow][columna_actual]] == '\n' || (*buffer)[pantalla[actualRow][columna_actual]] == '\0'){
        caracter_temp = CARACTER_DE_CONTROL; // En caso de ser EOL o salto de línea, usa un caracter de control.
      }else{
        // En cualquier otro caso, muestra el caracter presente en el indice almacenado en pantalla.
        caracter_temp = (*buffer)[pantalla[actualRow][columna_actual]];
      }
      mvprintw(actualRow + 1, columna_actual, "%c", caracter_temp); // Imprime el caracter correspondiente en la coordenada actual.

      // Si es la posicion de cursor, desactiva el modo invertido para imprimir texto.
      if (actualRow == pos_y && columna_actual == pos_x){
        attroff(A_REVERSE);
      }
    }
  }
}

void showStatistics(char *fileName, char **buffer, size_t bufferSize, int pos_x, int pos_y, int pos_buffer, int maxCols){
  int lineas, linea_actual, pos_buffer_temp;

  // Bucle para contar el numero de líneas presentes en editor actual.
  for (pos_buffer_temp = 0, lineas = 0, linea_actual = 0; pos_buffer_temp < ((int)bufferSize); pos_buffer_temp++){
    // Para saber la linea actual, contabiliza unicamente aquellas antes de la posicion actual en el buffer.
    if (pos_buffer_temp < pos_buffer){
      linea_actual += (*buffer)[pos_buffer_temp] == '\n';
    }
    lineas += (*buffer)[pos_buffer_temp] == '\n'; // Contabilizacion de líneas generales.
  }

  // Muestra el menu del editor
  attron(A_REVERSE);
  mvhline(0, 0, '#', maxCols);
  mvprintw(0, 0, "Archivo: \"%s\" %s || ESC = Salir || Lineas: %i || Linea Actual: %i || Col: %i | Fila: %i | Pos. Buffer: %i", fileName, checkFileExists(fileName) ? "" : "(Nuevo)", lineas, linea_actual, pos_x, pos_y, pos_buffer);
  attroff(A_REVERSE);
}
