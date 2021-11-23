#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>					
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wait.h>
#include "../include/carrito.h"
#include "include/systemv.h"

// Variables Globales
lista catalogo,cartClient;
carrito auxList;
unsigned int server,menu,submenu,pos,i,j,n,fd,condicional,totalClient,auxCant,passAdmin,passProveedor;
char auxString[21];
unsigned status;


int main(void)
{
    int sem_servidor;
    int sem_proceso;
    int *comunicacion;
    int clave, archivo;

    /* ------------------------- Memorias compartidas ------------------------- */

    /* Conexión a la memoria compartida de comunicación con el servidor */
    comunicacion = (int *)shm(sizeof(int)*3, "shm_comunicacion", 'v');

    /* ------------------------------ Semáforos ------------------------------- */

    /* Semáforo del servidor */
    sem_servidor = sem(0, "sem_servidor", 'w');

    /* Semáforo del cliente */
    sem_proceso = sem(0, "sem_proceso", 'x');

    /* -------------------- Comunicación con el servidor ---------------------- */

    comunicacion[0] = 3; // Pasamos el tipo de proceso al servidor

    up(sem_servidor);
    printf("Obteniendo llaves...\n");
    down(sem_proceso);

    clave = comunicacion[1];
    archivo = comunicacion[2];

    up(sem_servidor);

    printf("Llaves obtenidas...\n");
    printf("Clave: %c, Archivo: %d\n", clave, archivo);
    
     //Se crean las listas a utilizar
	crearlista(&catalogo); // Catalogo de Productos de la Tienda
    

	do{// Comienza Menu Principal del Cliente
		printf("\t*************** Proyecto Tienda Online ***************\n\n");
		printf ("********************** MENU PRINCIPAL - Cliente *************************\n\n"); 
		printf("!!! Bienvenido a Escommerce !!!\n\n");
		printf("1) Mostrar Catalogo de Tienda\n2) Gestionar Carrito\n0) Salir de la Cuenta Cliente\n\n");	
		printf("Ingrese el NUMERO de la opcion deseada y presione ENTER ~& ");
		scanf("%i",&menu);
	
		switch (menu){ // Operaciones para el MENU PRINCIPAL
			case 1: // 1) Mostrar Catalogo
				if(!empty(catalogo)){ // Verifica si el catalogo no esta vacio
					printf ("\n---- Catalogo de Productos Disponibles ----\n");    
					for(i=0;i<catalogo->NE;i++){ // Se imprimen los elementos de la lista "Catalogo de Productos Disponibles"
						printf("Producto #%d: %s\n",i,get(i,catalogo).name);
						printf("Cantidad: %d Unidades.\n",get(i,catalogo).cant);
						printf("Precio: $%d Pesos.\n\n",get(i,catalogo).precio);   
					}
					printf("\n!!! Fin del Catalogo !!! Presione Enter para Continuar...");
				}else{
					printf("\n!!! El Catalogo esta vacio !!! Presione Enter para Continuar... ");
				}
				getchar(); getchar(); system("clear");
			break;

			case 2: // 2) Ver Carrito
				do{ // Inicia Submenu Gestionar Carrito
					system("clear");
					printf("\t*************** Proyecto Tienda Online ***************\n\n");
					printf ("********************** SUBMENU GESTIONAR CARRITO - Cliente *************************\n\n"); 
					printf("1) Ver Carrito \n2) Agregar un Producto al Carrito \n3) Borrar un Producto del Carrito \n0) Regresar al Menu Principal\n\n"); // Opciones 	
					printf("Ingrese el NUMERO de la operacion deseada y presione ENTER ~& ");
					scanf("%i",&submenu);
									
					switch(submenu){
						case 1: // Opcion Ver Carrito
							if (!empty(cartClient)){ // Carrito Tiene Productos
								for(i=0;i<cartClient->NE;i++){ // Se imprimen los elementos de la lista "Carrito"
									printf("Producto # %d: %s\n",i,get(i,cartClient).name);
									printf("Cantidad: %d Unidades.\n",get(i,cartClient).cant);
									printf("Precio: $%d Pesos.\n\n",get(i,cartClient).precio);   
								}
								printf("!!! Fin del Carrito !!! Presione Enter para Continuar... ");	  
							}else{ // Carrito esta Vacio
								printf("\n!!! El Carrito esta vacio !!! Presione Enter para continuar...");
							}	
							getchar(); getchar(); system("clear");
						break;
											
						case 2: // Opcion Agregar un Producto al Carrito 
							if(!empty(catalogo)){ // Verifica si el catalogo no esta vacio
								printf ("\n---- Catalogo de Productos Disponibles ----\n");    
								for(i=0;i<catalogo->NE;i++){ // Se imprimen los elementos de la lista "Catalogo de Productos Disponibles"
									printf("Producto # %d: %s\n",i,get(i,catalogo).name);
									printf("Cantidad: %d Unidades.\n",get(i,catalogo).cant);
									printf("Precio: $%d Pesos.\n\n",get(i,catalogo).precio);   
								}
								printf ("\n-------------------------------------------\n\n");

								printf("Ingrese el NUMERO de producto que desea agregar al carrito y presione ENTER (0 - %d) ~& ",catalogo->NE);
								scanf("%i",&i);

								if (i>=0 && i<=catalogo->NE){
											condicional=0;
									
									if(empty(cartClient)==FALSE){ // Se verifica si el Carrito NO esta vacio 
										strcpy(auxList.name,get(i,catalogo).name); // Se copia producto del catalogo en auxList
										auxList.cant=get(i,catalogo).cant;
										auxList.precio=get(i,catalogo).precio;

											for(j=0;j<cartClient->NE;j++){ // Se recorre el carrito en busca de existencia del mismo producto seleccionado en catalogo
												if(strcmp(auxList.name,get(j,cartClient).name)==0){ // Se encontro el mismo producto en carrito
													// Se actualiza cantidad de producto en carrito +1
													strcpy(auxList.name,get(j,cartClient).name);
													auxCant=get(j,cartClient).cant;
													auxCant++;
													auxList.cant=auxCant;
													auxList.precio=get(j,cartClient).precio;
													
													borrar (j,cartClient);
													add (j,auxList,cartClient);
													
													// Se actualiza cantidad de producto en Catalogo -1
													strcpy(auxList.name,get(i,catalogo).name);
													auxCant=get(i,catalogo).cant;
													auxCant--;
													auxList.cant=auxCant;
													auxList.precio=get(i,catalogo).precio;
													
													borrar (i,catalogo);
													if(auxList.cant>=1) add (i,auxList,catalogo); // Si aun queda producto suficiente en catalogo, se actualiza registro
													condicional=1; // si activo (1), no se agregara un segundo producto
												}
											}
										fflush(stdin);
									}

									if(condicional==0){ // Se verifica si el Producto es nuevo en carrito
										strcpy(auxList.name,get(i,catalogo).name);
										auxList.cant=1;
										auxList.precio=get(i,catalogo).precio;

										printf ("\n---- Catalogo de Productos Disponibles en CARRITO ----\n");    
										for(i=0;i<cartClient->NE;i++){ // Se imprimen los elementos de la lista "Carrito"
											printf("Producto # %d: %s\n",i,get(i,cartClient).name);
											printf("Cantidad: %d Unidades.\n",get(i,cartClient).cant);
											printf("Precio: $%d Pesos.\n\n",get(i,cartClient).precio);   
										}
										printf ("\n-------------------------------------------\n\n");

										fflush(stdin);
										printf("\nIngrese el NUMERO de la POSICION donde se desea agregar el producto a su carrito y presione ENTER. (0 - 50)\n");
										printf ("* (Si selecciona una posicion ocupada, el nuevo producto se colocara ahi y los demas productos se recorreran un espacio.) ~& ");
										scanf("%i",&pos); //Se ingresa la posicion
							
										if (pos>=0){
											add (pos,auxList,cartClient); // Se agrega producto a carrito

											//se resta en 1 la cantidad del producto en el catalogo
											strcpy(auxList.name,get(i,catalogo).name);
											auxCant=get(i,catalogo).cant;
											auxCant--;
											auxList.cant=auxCant;
											auxList.precio=get(i,catalogo).precio;							
											borrar (i,catalogo);
											if(auxList.cant>=1) add (i,auxList,catalogo); // Si aun queda producto suficiente en catalogo, se actualiza registro
											printf("\n!! Producto Agregado al Carrito !! Presione ENTER para Continuar... ");
										}else{
											printf("\n!!! Ha insertado un rango de Posicion invalida!!! Presione Enter para Intentar de nuevo... "); 
										}
									}
									condicional=0;
									fflush(stdin);

								} else{ // Opcion invalida en catalogo
									printf("\n!!! Ha insertado una opcion invalida!!! Presione Enter para Intentar de nuevo... "); 
								}
							}else { // El Catalogo esta vacio
								printf("\n!!! El Catalogo esta vacio !!! Presione Enter para continuar... ");
							}
							getchar(); getchar(); system("clear");
						break;

						case 3: // Opcion Borrar un Producto del carrito
							if (!empty(cartClient)){ // Se comprueba si no esta vacio el carrito
								printf ("\n---- Catalogo de Productos Disponibles en CARRITO ----\n");    
								for(i=0;i<cartClient->NE;i++){ // Se imprimen los elementos de la lista "Carrito"
									printf("Producto # %d: %s\n",i,get(i,cartClient).name);
									printf("Cantidad: %d Unidades.\n",get(i,cartClient).cant);
									printf("Precio: $%d Pesos.\n\n",get(i,cartClient).precio);   
								}
								printf ("\n-------------------------------------------\n\n");
								printf("\nSeleccione la NUMERO del producto que desea Borrar del Carrito (0 - %d) ~& \n",cartClient->NE);
								scanf("%i",&pos);//Se ingresa la posicion
								if (pos>=0 && pos<cartClient->NE){
									borrar (pos,cartClient); // Se borra producto de carrito
									printf("\n!!! Producto borrado del carrito !!! Presione Enter para Continuar... ");
								} else {
									printf("\n!!! Numero de Producto inexistente !!! Presione Enter para Intentar de Nuevo... ");
								}												
							}else{
								printf("\n!!! El Carrito esta vacio !!! Presione Enter para Continuar... ");
							}
							getchar(); getchar(); system("clear");
						break;

						case 0: system("clear"); break; // Regresar al Menu Principal
											
						default: printf("\n!!! Ha insertado una opcion invalida !!! Presione Enter para Intentar de nuevo... "); getchar(); getchar(); system("clear");
					} //Termina switch submenu
				
				}while(submenu!=0); system("clear");
			break;
			case 0: // Mensaje al salir del programa
				printf("\n!!! Hasta la proxima !!! Presione Enter para Continuar... "); getchar(); getchar(); system("clear"); exit(0);
			break;

			default: printf("\n!!! Ha insertado una opcion invalida !!! Presione Enter para Intentar de nuevo...\n"); getchar(); getchar(); system("clear");	
		}
	}while(menu!=0);
	
		liberarlista (&catalogo);
	
    return 0;
}
