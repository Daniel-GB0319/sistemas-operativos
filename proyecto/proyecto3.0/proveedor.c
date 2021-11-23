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

    comunicacion[0] = 2; // Pasamos el tipo de proceso al servidor

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

	do{// Comienza Menu Principal del Proveedor
		printf("\t*************** Proyecto Tienda Online ***************\n\n");
		printf ("********************** MENU PRINCIPAL - Proveedor *************************\n\n"); 
		printf("!!! Bienvenido a Escommerce !!!\n\n");
		printf("1) Mostrar Catalogo de Tienda\n2) Gestionar Productos Ofertados en Catalogo \n0) Salir de la Cuenta Proveedor\n\n");	
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
				do{ // Inicia Submenu Gestionar Productos Ofertados en Catalogo
					system("clear");
					printf("\t*************** Proyecto Tienda Online ***************\n\n");
					printf ("********************** SUBMENU GESTIONAR CARRITO - Catalogo *************************\n\n"); 
					printf("1) Ver Catalogo \n2) Agregar un Producto al Catalogo \n3) Borrar un Producto del Catalogo \n0) Regresar al Menu Principal\n\n"); // Opciones 	
					printf("Ingrese el NUMERO de la operacion deseada y presione ENTER ~& ");
					scanf("%i",&submenu);
									
					switch(submenu){
						case 1: // Opcion Ver Catalogo
							if (!empty(catalogo)){ // Catalogo Tiene Productos
								printf ("\n---- Catalogo de Productos Disponibles ----\n");    
								for(i=0;i<catalogo->NE;i++){ // Se imprimen los elementos de la lista "Catalogo de Productos Disponibles"
									printf("Producto #%d: %s\n",i,get(i,catalogo).name);
									printf("Cantidad: %d Unidades.\n",get(i,catalogo).cant);
									printf("Precio: $%d Pesos.\n\n",get(i,catalogo).precio);   
								}
								printf("\n!!! Fin del Catalogo !!! Presione Enter para Continuar... ");	  
							}else{ // Catalogo esta Vacio
								printf("\n!!! El Catalogo esta vacio !!! Presione Enter para continuar... ");
							}	
							getchar(); getchar(); system("clear");
						break;
											
						case 2: // Opcion Agregar un Producto al Catalogo
								printf ("Ingrese la CONTRASEÑA de Proveedor y presione Enter (6 Digitos) ~& ");
								scanf("%i",&i);
								if (i>=100000 && i<=999999){ // Contraseña ingresada dentro del rango permitido
									
									if(i==passProveedor){ // Contraseña de Proveedor ingresada correctamente
										printf ("\n---- Catalogo de Productos Disponibles ----\n");    
										for(i=0;i<catalogo->NE;i++){ // Se imprimen los elementos de la lista "Catalogo de Productos Disponibles"
											printf("Producto # %d: %s\n",i,get(i,catalogo).name);
											printf("Cantidad: %d Unidades.\n",get(i,catalogo).cant);
											printf("Precio: $%d Pesos.\n\n",get(i,catalogo).precio);   
										}
										printf ("\n-------------------------------------------\n\n");

										getchar();
										printf("\nIngrese el Nombre del Nuevo Producto y presione ENTER. (Max. 20 Caracteres) ~& ");
										scanf("%20[^\n]s",auxString); //Se ingresa el nombre del Nuevo Producto
										getchar();

										printf("\nIngrese la Cantidad del Nuevo Producto y presione ENTER. (1 - 999) ~& ");
										scanf("%d",&j); //Se ingresa la cantidad del Nuevo Producto
										 
										if (j>=1 && j<=999){ // Verifica si cantidad esta dentro del rango permitido
											printf("\nIngrese el Precio del Nuevo Producto y presione ENTER. (1 - 99999) ~& ");
											scanf("%d",&i); //Se ingresa la cantidad del Nuevo Producto
											
											if (i>=1 && i<=99999){ // Verifica si Precio esta dentro del rango permitido
												strcpy(auxList.name,auxString);
												auxList.cant=j;
												auxList.precio=i;

												printf("\nIngrese la POSICION donde desea colocar el Nuevo Producto y presione ENTER. (0 - 50)\n");
												printf ("* (Si selecciona una posicion ocupada, el nuevo producto se colocara ahi y los demas productos se recorreran un espacio.) ~& ");
												scanf("%i",&pos); //Se ingresa la posicion
									
												if (pos>=0){
													add (pos,auxList,catalogo); // Se agrega producto a Catalogo
													printf("\n!! Producto Agregado al Catalogo !! Presione ENTER para Continuar... ");
												}else{
													printf("\n!!! Ha insertado un rango de Posicion invalida!!! Presione Enter para Intentar de nuevo... "); 
												}

											}else{ // Precio Ingresado fuera de Rango
												printf("\n!!! Ha insertado un Precio invalido !!! Presione Enter para Intentar de nuevo... ");
											}

										}else{ // Cantidad Ingresada fuera de Rango
											printf("\n!!! Ha insertado una Cantidad invalida!!! Presione Enter para Intentar de nuevo... ");
										}
										
									}else{ // Contraseña de Proveedor Incorrecta
										printf("\n!!! Contraseña de Proveedor Incorrecta !!! Presione Enter para Intentar de Nuevo... "); 
									}
								
								}else { //Contraseña ingresada Fuera de Rango
									printf("\n!!! La Contraseña Ingresada esta fuera de Rango !!! Presione Enter para Intentar de Nuevo... "); 
								}
							
							getchar(); getchar(); system("clear");
						break;

						case 3: // Opcion Borrar un Producto del Catalogo
							if (!empty(catalogo)){ // Se comprueba si no esta vacio el carrito
								printf ("\n---- Catalogo de Productos Disponibles ----\n");    
								for(i=0;i<catalogo->NE;i++){ // Se imprimen los elementos de la lista "Catalogo de Productos Disponibles"
									printf("Producto # %d: %s\n",i,get(i,catalogo).name);
									printf("Cantidad: %d Unidades.\n",get(i,catalogo).cant);
									printf("Precio: $%d Pesos.\n\n",get(i,catalogo).precio);   
								}
								printf ("\n-------------------------------------------\n\n");

								printf("\nSeleccione la NUMERO del producto que desea Borrar del Catalogo (0 - %d) ~& \n",catalogo->NE);
								scanf("%i",&pos);//Se ingresa la posicion
								if (pos>=0 && pos<catalogo->NE){
									borrar (pos,catalogo); // Se borra producto de catalogo
									printf("\n!!! Producto borrado del Catalogo !!! Presione Enter para Continuar... ");
								} else {
									printf("\n!!! Numero de Producto inexistente !!! Presione Enter para Intentar de Nuevo... ");
								}												
							}else{
								printf("\n!!! El Catalogo esta vacio !!! Presione Enter para Continuar... ");
							}
							getchar(); getchar(); system("clear");
						break;

						case 0: system("clear"); break; // Regresar al Menu Principal
											
						default: printf("\n!!! Ha insertado una opcion invalida !!! Presione Enter para Intentar de nuevo... "); getchar(); getchar(); system("clear");
					} //Termina switch submenu
				
				}while(submenu!=0); system("clear");
			break;
			
			case 0: // Mensaje al salir del programa
				printf("\n!!! Hasta la proxima !!! Presione Enter para Continuar... "); getchar(); getchar(); system("clear"); main();
			break;

			default: printf("\n!!! Ha insertado una opcion invalida !!! Presione Enter para Intentar de nuevo...\n"); getchar(); getchar(); system("clear");	
		}
	}while(menu!=0);

		liberarlista (&catalogo);

    return 0;
}
