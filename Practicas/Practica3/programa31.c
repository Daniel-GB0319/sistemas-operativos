#include <stdio.h>  
#include <stdlib.h> 
#include <time.h> 
#include <unistd.h>  

#define NUM_PAGINAS 16
#define NUM_MARCOS 8

typedef struct marcoPagina{ // Estructura de los marcos de pagina
  int idMarco;                 
  int idPage;                 
  struct marcoPagina *next;      
} marcoBase;

// Auxiliar en busqueda y asignacion de paginas en memoria
marcoBase *findPageMemory(int searchIdPage, marcoBase *firstPageMarco){
  marcoBase *aux = firstPageMarco;

  while (aux != NULL){
    if (searchIdPage == aux->idPage){
      return aux;
    }
    aux = aux->next;
  }
  return NULL;
}

// Muestra la tabla de marcos de paginas
void marcoTable(marcoBase *initMarcoPage){
  marcoBase *aux = initMarcoPage;

  printf("\n Tabla de Marcos de Pagina:\n\n");
  printf(" ** ID Marco de Pagina ----- ID Pagina **\n\n");
  while (aux != NULL){
    printf(" * %18i ----- %9i *\n", aux->idMarco, aux->idPage);
    aux = aux->next;
  }
  printf("!! Fin de la Tabla de Marcos de pagina !! Presione Enter para Continuar ...\n");
  getchar();
  getchar();
}

// Inserta pagina en Memoria
void insertPageMemory(int insertPageId, marcoBase **initMarcoPage){
  marcoBase *temp,                                   
  *newMarcoPage = malloc(sizeof(marcoBase));   
  int elemCount;                                           

  newMarcoPage->next = NULL;
  newMarcoPage->idPage = insertPageId;

  temp = findPageMemory(insertPageId, *initMarcoPage);
  if (temp != NULL){
    return;
  }
  elemCount = 0;
  temp = *initMarcoPage;

  while (temp != NULL){
    elemCount++;
    temp = temp->next;
  }

  if (elemCount < NUM_MARCOS){
    newMarcoPage->idMarco = elemCount;
    newMarcoPage->idPage = insertPageId;
    if (elemCount == 0){
      (*initMarcoPage) = newMarcoPage;
    }else{
      temp = *initMarcoPage;
      while (temp->next != NULL){
        temp = temp->next;
      }
      temp->next = newMarcoPage;
    }
  }else{
    newMarcoPage->idMarco = (*initMarcoPage)->idMarco;
    (*initMarcoPage) = (*initMarcoPage)->next;
    temp = (*initMarcoPage);
    while (temp->next != NULL){
      temp = temp->next;
    }
    temp->next = newMarcoPage;
  }
  printf("\n !! La pagina con Id = %i ahora se encuentra en la Memoria con el Id = %i !!\n", insertPageId, newMarcoPage->idMarco);
}

// Muestra la tabla de paginas
void pageTable(marcoBase *initMarcoPage){
  marcoBase *foundMarcoPage = NULL;   
  int i;                                

  printf("\nTabla de las paginas:\n\n");
  printf(" ** ID de Pagina ----- ID de Marco de Pagina **\n\n");

  for (i = 0; i < NUM_PAGINAS; i++){
    foundMarcoPage = findPageMemory(i, initMarcoPage);

    if (foundMarcoPage != NULL){
      printf(" * %9i ----- %18i *\n", i, foundMarcoPage->idMarco);
    }else{
      printf(" * %9i ----- Memoria Virtual *\n", i);
    }
  }
  printf("!! Fin de la Tabla de las paginas !! Presione Enter para Continuar ...\n");
  getchar();
  getchar();
}

// FUNCION PRINCIPAL
int main(int argc, char *argv[]){
  // Se inicializa Memoria Fisica
  marcoBase *initMarcoPage = NULL;       
  int menu, i, temp, insertPageId;                                                                  

  srand(time(NULL));
  // Se cargan paginas aleatoriamente a la memoria fisica
    for (i = 0; i < NUM_MARCOS; i++){
      do{
        temp = rand() % NUM_PAGINAS;
      } while (findPageMemory(temp, initMarcoPage) != NULL);
      insertPageMemory(temp, &initMarcoPage);
    }
 
  do{
    //system ("clear");
	system ("cls");
    printf("\n*** Practica 3 -- Simulador de Memoria Virtual ***\n\n");
    printf("---- Menu Principal ----\n\n");
    printf(" 1) Ver la tabla de paginas.\n");
    printf(" 2) Ver la tabla de marcos de pagina.\n");
    printf(" 3) Insertar pagina en Memoria.\n");
    printf(" 0) Salir del programa.\n\n");

    printf(" Ingrese la opcion deseada y presione Enter -$ ");
    scanf("%i", &menu);
    
    switch (menu){
        case 1: // Tabla de paginas
            pageTable(initMarcoPage);
        break;

        case 2: // Tabla de Marcos de Pagina
            marcoTable(initMarcoPage);
        break;

        case 3: // Se decide ingresar una pagina en memoria fisica
            printf("\nIngrese el ID de la pagina a insertar en la Memoria y presiones Enter (0 - 15) -$ \n");
            scanf("%i", &insertPageId); 
            if (insertPageId >=0 && insertPageId <=15){
              insertPageMemory(insertPageId, &initMarcoPage);
              getchar();
              getchar();
            }else{
              printf (" !! Ha ingresado una opcion invalida !! Intente de nuevo .. \n");
              getchar();
              getchar();
              //system ("clear");
              system ("cls");
            }    
        break;

        case 0:
            printf("\n !! Gracias por utilizar el programa !! Hasta la Proxima.\n");
        break;

        default:
          printf(" !! Ha insertado una opcion invalida !! Intente de nuevo ..\n");
          getchar();
          getchar();
          //system ("clear");
          system ("cls");
        break;
    }
  } while (menu != 0); 
  return 0;
}
