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
#include "../include/systemv.h"

// Prototipos
void atender_cliente(void *argumentos);
void menuAdmin();
void menuCliente();
void menuProveedor();

// Variables Globales
lista catalogo,cartClient;
carrito auxList;
unsigned int server,menu,submenu,pos,i,j,n,fd,condicional,totalClient,auxCant,passAdmin,passProveedor;
char auxString[21], *contents = NULL;
unsigned status;
FILE* fileCatalogo;

int main(void)
{
    int contador;
    int sem_servidor;
    int sem_cliente;
    int sem_proveedor;
	int sem_administrador;
    int llaves_conexion[20][2];
    char numero_acceso[3]; // Para convertir el número de asiento a cadena
    char archivo_acceso[15]; // Para alamcenar el nombre del archivo para la llave de cada asiento
    int *acceso = NULL;
    int *parametros_cliente = NULL;
    int *parametros_proveedor = NULL;
	int *parametros_administrador = NULL;

    /* ------------------------- Memorias compartidas ------------------------- */

    /* Arreglo de acceso */
    acceso = (int *)shm(sizeof(int)*20, "shm_acceso", 'u');

    /** 
     * Memoria compartida para asignar llaves de comunicación al cliente
     * nuevo para que pueda comunicarse con el hilo que le es asignado
     */
    parametros_cliente = (int *)shm(sizeof(int)*2, "shm_parametros_cliente", 'v');
     /** 
     * Memoria compartida para asignar llaves de comunicación al proveedor
     * nuevo para que pueda comunicarse con el hilo que le es asignado
     */
    parametros_proveedor = (int *)shm(sizeof(int)*2, "shm_parametros_proveedor", 'v');
	/** 
     * Memoria compartida para asignar llaves de comunicación al administrador
     * nuevo para que pueda comunicarse con el hilo que le es asignado
     */
    parametros_administrador = (int *)shm(sizeof(int)*2, "shm_parametros_administrador", 'v');

    /* ------------------------------ Semáforos ------------------------------- */

    /* Semáforo del servidor */
    sem_servidor = sem(0, "sem_servidor", 'w');

    /* Semáforo del cliente */
    sem_cliente = sem(0, "sem_cliente", 'x');

    /* Semáforo del proveedor */
    sem_proveedor = sem(0, "sem_proveedor", 'x');

	/* Semáforo del administradorr */
    sem_administrador = sem(0, "sem_administrador", 'x');

    /* Llenado del arreglo de acceso */
    for (contador = 0; contador < 20; contador++) {
        /* Creación del nombre del archivo de la clave del acceso en el índice @contador */
        sprintf(numero_acceso, "%d", contador+1);
        strcpy(archivo_acceso, "sem_acceso_");
        strcat(archivo_acceso, numero_acceso);

        /**
         * Creación del semáforo para el acceso en el índice @contador.
         * 
         * Ya que los acceso son administrados por medio de un arreglo de enteros,
         * podemos utilizar cada elemento del arreglo como semáforo del acceso
         * correspondiente en el índice @contador.
         */
        acceso[contador] = sem(1, archivo_acceso, contador+100);
    }

    /* --------------------------- Configuraciones ---------------------------- */

    /* Llenado del arreglo con las llaves de conexión para la memoria compartida con hilos */
    for (contador = 0; contador < 20; contador++) {
        llaves_conexion[contador][0] = contador + 97;
        llaves_conexion[contador][1] = true;
    }

    /* Hilos para atender */
    pthread_attr_t atributos;
	pthread_t hilos[20];
	pthread_attr_init (&atributos);
	pthread_attr_setdetachstate (&atributos, PTHREAD_CREATE_DETACHED);

	

    //Se crean las listas a utilizar
	crearlista(&catalogo); // Catalogo de Productos de la Tienda
	crearlista(&cartClient);
	
	passAdmin=123456;
	passProveedor=123456;

	do{ // Inicia SERVIDOR
		system("clear");
		printf("\t*************** Proyecto Tienda Online ***************\n\n");
		printf ("********************** INICIO *************************\n\n"); 
		printf("!!! Bienvenido a Escommerce !!!\n\n");
		printf("1) Acceder como Administrador \n2) Realizar Conexion con un Cliente \n3) Realizar Conexion con un Proveedor\n0) Salir de la Tienda\n\n");	
		printf("Ingrese el NUMERO de la opcion deseada y presione ENTER ~& ");
		scanf("%i",&server);

		switch (server){
			case 1: // MENU ADMIN

			while (1) {
                    printf("Esperando conexion...\n");
                    down(sem_servidor);
                    printf("Conexion realizada, creando hilo...\n");

                    for (contador = 0; contador < 20; contador++) {
                        if (llaves_conexion[contador][1]) {
                            parametros_administrador[0] = llaves_conexion[contador][0];
                            parametros_administrador[1] = contador+200;
                            llaves_conexion[contador][1] = false;
                            break;
                        }
                    }

                    up(sem_administrador);
                    down(sem_servidor);

                    parametros_administrador[0] = 'x';
                    parametros_administrador[1] = 'x';

				printf("\nIngrese la contraseña de Administrador (Si es ejecucion por primera vez, ingrese 123456 ) ~& ");
				scanf("%d",&i);
				if(i>=100000 && i<=999999){ // Contraseña Ingresada dentro del rango permitido
					if (i == passAdmin){
					menuAdmin();
					}else{
						printf("\n!!! Contraseña Incorrecta !!! Presione Enter para Intentar de Nuevo... "); 
						getchar(); getchar(); system("clear"); main();
					}
				}else{ // Contraseña ingresada fuera del rango permitido
					printf("\n!!! Contraseña Ingresada fuera del rango permitido !!! Presione Enter para Intentar de Nuevo... ");
					getchar(); getchar(); system("clear"); main();
				}
			}
			break;

			case 2: 
				while (1) {
                    printf("Esperando conexion...\n");
                    down(sem_servidor);
                    printf("Conexion realizada, creando hilo...\n");

                    for (contador = 0; contador < 20; contador++) {
                        if (llaves_conexion[contador][1]) {
                            parametros_cliente[0] = llaves_conexion[contador][0];
                            parametros_cliente[1] = contador+200;
                            llaves_conexion[contador][1] = false;
                            break;
                        }
                    }

                    up(sem_cliente);
                    down(sem_servidor);

                    parametros_cliente[0] = 'x';
                    parametros_cliente[1] = 'x';

                    menuCliente();
                   
                }
			break;

            case 3:// Conexion Proveedor
            while (1) {
                    printf("Esperando conexion...\n");
                    down(sem_servidor);
                    printf("Conexion realizada, creando hilo...\n");

                    for (contador = 0; contador < 20; contador++) {
                        if (llaves_conexion[contador][1]) {
                            parametros_proveedor[0] = llaves_conexion[contador][0];
                            parametros_proveedor[1] = contador+200;
                            llaves_conexion[contador][1] = false;
                            break;
                        }
                    }

                    up(sem_proveedor);
                    down(sem_servidor);

                    parametros_proveedor[0] = 'x';
                    parametros_proveedor[1] = 'x';
                    
                    menuProveedor();
            }
            break;
			case 0:
				
				printf("!!! Gracias por Utilizar Escommerce !!! Hasta la Proxima \n\n");
				free(contents);
				exit(0);

			break;
		
			default:
				printf("\n!!! Ha insertado una opcion invalida !!! Presione Enter para Intentar de Nuevo..."); 
				getchar(); getchar(); system("clear");
			break;
		}

	} while (server!=0);
	
	liberarlista (&catalogo);
	liberarlista (&cartClient);
	return 0;
}


// ############################################################ FUNCIONES #######################################################################


// **************************************************************************** ADMINISTRADOR *******************************************************************************************
//***************************************************************************************************************************************************************************************
void menuAdmin(){

	// Se abre Fichero
	fileCatalogo = fopen("Catalogo.txt","r+");
	if(fileCatalogo==NULL){
		printf("\n!!! No se ha podido acceder al archivo Catalogo.txt !!!\n ");
		exit(-1); 
	}

	//Se precargan productos en la lista "Catalogo de Productos"
    size_t len = 0;
	pos=0;
    while (getline(&contents, &len, fileCatalogo) != -1){
        strcpy(auxList.name,contents);
		getline(&contents, &len, fileCatalogo);
		auxList.cant=atoi(contents);
		getline(&contents, &len, fileCatalogo);
		auxList.precio=atoi(contents);

		add (pos,auxList,catalogo);
		pos++;
		//printf("%s", contents);
    }
	pos=0;
	fclose(fileCatalogo);

	do{// Comienza Menu Principal del Programa
		system("clear");
		printf("\t*************** Proyecto Tienda Online ***************\n\n");
		printf ("********************** MENU PRINCIPAL - Administrador *************************\n\n"); 
		printf("!!! Bienvenido a Escommerce !!!\n\n");
		printf("1) Mostrar Catalogo de Tienda \n2) Gestionar Catalogo\n");
		printf("3) Gestionar Contraseñas \n0) Salir de la Cuenta de Adminitrador\n\n");	
		printf("Ingrese el NUMERO de la opcion deseada y presione ENTER ~& ");
		scanf("%i",&menu);
	
		switch (menu){ // Operaciones para el MENU PRINCIPAL
			case 1: // 1) Mostrar Catalogo
				printf ("\n---- Catalogo de Productos Disponibles ----\n\n");    
				for(i=0;i<catalogo->NE;i++){ // Se imprimen los elementos de la lista "Catalogo de Productos Disponibles"
	  				printf("Producto #%d: %s\n",i,get(i,catalogo).name);
					printf("Cantidad: %d Unidades.\n",get(i,catalogo).cant);
					printf("Precio: $%d Pesos.\n\n",get(i,catalogo).precio);   
				}
				printf("!!! Fin del Catalogo !!! Presione Enter para Continuar... ");	  
				getchar(); getchar(); system("clear");
			break;
			
			case 2: // 2) GESTIONAR CATALOGO
				do{ // Inicia Submenu Gestionar Catalogo
					system("clear");
					printf("\t*************** Proyecto Tienda Online ***************\n\n");
					printf ("********************** SUBMENU GESTIONAR CATALOGO - Admin *************************\n\n"); 
					printf("1) Ver Catalogo \n2) Reubicar un Producto del Catalogo \n");
					printf("3) Borrar un Producto del Catalogo \n4) Limpiar Contenido del Catalogo \n0) Regresar al Menu Principal\n\n"); // Opciones 	
					printf("Ingrese el NUMERO de la operacion deseada y presione ENTER ~& ");
					scanf("%i",&submenu);
									
					switch(submenu){
						case 1: // Opcion Ver Catalogo
							if (!empty(catalogo)){ // Carrito Tiene Productos
								printf ("\n---- Catalogo de Productos Disponibles ----\n");    
								for(i=0;i<catalogo->NE;i++){ // Se imprimen los elementos de la lista "Catalogo de Productos Disponibles"
									printf("Producto # %d: %s\n",i,get(i,catalogo).name);
									printf("Cantidad: %d Unidades.\n",get(i,catalogo).cant);
									printf("Precio: $%d Pesos.\n\n",get(i,catalogo).precio);   
								}
								printf ("\n-------------------------------------------\n\n");
								printf("!!! Fin del Catalogo !!! Presione Enter para Continuar...");	  
							}else{ // Catalogo esta Vacio
								printf("\n!!! El Catalogo esta vacio !!! Presione Enter para continuar..."); 	
							}	
							getchar(); getchar(); system("clear");
						break;
											
						case 2: // Opcion Reubicar un producto del catalogo 
							if(empty(catalogo)==FALSE){ // Se verifica si el catalogo No esta vacio
								printf ("\n---- Catalogo de Productos Disponibles ----\n");    
								for(i=0;i<catalogo->NE;i++){ // Se imprimen los elementos de la lista "Catalogo de Productos Disponibles"
									printf("Producto # %d: %s\n",i,get(i,catalogo).name);
									printf("Cantidad: %d Unidades.\n",get(i,catalogo).cant);
									printf("Precio: $%d Pesos.\n\n",get(i,catalogo).precio);   
								}
								printf ("\n-------------------------------------------\n\n");

								printf("Ingrese el NUMERO del producto que desea reubicar dentro del catalogo  y presione ENTER (0 - %d) ~& ",catalogo->NE);
								scanf("%i",&i);

								if (i>=0 && i<=catalogo->NE){
									strcpy(auxList.name,get(i,catalogo).name);
									auxList.cant=get(i,catalogo).cant;
									auxList.precio=get(i,catalogo).precio;
									
									printf("\nIngrese el NUMERO de la POSICION donde se desea agregar el producto a su carrito y presione ENTER. (0 - 50)\n");
									printf ("* (Si selecciona una posicion ocupada, el nuevo producto se colocara ahi y los demas productos se recorreran un espacio.) ~& ");
									scanf("%i",&pos); //Se ingresa la posicion
						
									if (pos>=0){ // Se ingresan valores validos para realizar reubicacion 
										borrar (i,catalogo); // Se elimina producto de su posicion original
										add (pos,auxList,catalogo); // Se agrega producto a nueva posicion en catalogo
										printf("\n!! Producto Reubicado con Exito !! Presione ENTER para Continuar... ");
									}else { // Se ingresa una posicion invalida
										printf("\n!!! Ha insertado un rango de Posicion invalida!!! Presione Enter para Intentar de nuevo... "); 
									}
									
								}else { // Se ingreso un numero de producto invalido para el catalogo 
									printf("\n!!! Ha ingresado un Numero de producto invalido!!! Presione Enter para Intentar de nuevo... "); 
								}
							}else { // Catalogo Vacio
								printf("\n!!! El Catalogo esta Vacio !!! Presione Enter para Intentar de nuevo... "); 
							}							
							getchar(); getchar(); system("clear");
						break;

						case 3: // Opcion Borrar un Producto del Catalogo
							if (!empty(catalogo)){ // Se comprueba si no esta vacio el carrito
							// Existen productos en catalogo
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
									printf("\n!!! Producto borrado del catalogo !!! Presione Enter para Continuar... ");
								} else { // Se ingresa una posicion invalida
									printf("\n!!! Numero de Producto invalido !!! Presione Enter para Intentar de Nuevo... ");
								}												
							}else{ // Catalogo Vacio
								printf("\n!!! El Catalogo esta vacio !!! Presione Enter para Continuar... "); 
							}
							getchar(); getchar(); system("clear");
						break;
						
						case 4: // Opcion Limpiar contenido del Catalogo
							if (!empty(catalogo)){ // Se comprueba si no esta vacio el carrito
							// Existen productos en catalogo
								printf ("\n---- Catalogo de Productos Disponibles ----\n");    
								for(i=0;i<catalogo->NE;i++){ // Se imprimen los elementos de la lista "Catalogo de Productos Disponibles"
									printf("Producto # %d: %s\n",i,get(i,catalogo).name);
									printf("Cantidad: %d Unidades.\n",get(i,catalogo).cant);
									printf("Precio: $%d Pesos.\n\n",get(i,catalogo).precio);   
								}
								printf ("\n-------------------------------------------\n\n");
								
								printf ("Ingrese la CONTRASEÑA de Administrador y presione Enter (6 Digitos) ~& ");
								scanf("%i",&i);
								if (i>=100000 && i<=999999){ // Contraseña ingresada dentro del rango permitido
									if(i==passAdmin){ // Contraseña de admin ingresada correctamente
										printf("\n ¿Desea Borrar TODO el contenido del Catalogo? (SI=1 / NO=0)\n");
										printf ("Ingrese el numero de la opcion desea y presione Enter ~& ");
										scanf("%i",&i);//Se ingresa la opcion
										
										if(i==1){ // Se confirma la limpieza de todo el catalogo
											vaciarlista(catalogo);	
											printf("\n!!! El Catalogo se ha vaciado exitosamente !!! Presione Enter para Continuar... "); 
										}else{ // Operacion de Limpieza total cancelada por admin
											printf("\n!!! Operacion de Vaciado Cancelado !!! Presione Enter para Continuar... "); 
										}
										
									}else{ // Contraseña de admin Incorrecta
										printf("\n!!! Contraseña de Administrador Incorrecta !!! Presione Enter para Intentar de Nuevo... "); 
									}
								
								}else { //Contraseña ingresada Fuera de Rango
									printf("\n!!! La Contraseña Ingresada esta fuera de Rango !!! Presione Enter para Intentar de Nuevo... "); 
								}												
							}else{ // Catalogo Vacio
								printf("\n!!! El Catalogo esta vacio !!! Presione Enter para Continuar... "); 
							}	
							getchar(); getchar(); system("clear");
						break;
						
						case 0: system("clear"); break; // Regresar al Menu Principal					
						default: printf("\n!!! Ha insertado una opcion invalida !!! Presione Enter para Intentar de nuevo... "); getchar(); getchar(); system("clear");
					} //Termina switch submenu
				
				}while(submenu!=0);
			break;
			
			case 3: // Opcion Gestionar Contraseñas
				do{
					// Inicia Submenu Gestionar Contraseñas
					system("clear");
					printf("\t*************** Proyecto Tienda Online ***************\n\n");
					printf ("********************** SUBMENU GESTIONAR CONTRASEÑAS - Admin *************************\n\n"); 
					printf("1) Ver Contraseñas Almacenadas \n2) Cambiar contraseña de Administrador \n");
					printf("3) Cambiar Contraseña de Proveedor \n4) Reestablecer Contraseñas \n0) Regresar al Menu Principal\n\n"); // Opciones 	
					printf("Ingrese el NUMERO de la operacion deseada y presione ENTER ~& ");
					scanf("%i",&submenu);
					
					switch (submenu){
						case 1: // Opcion Ver Contraseñas Almacenadas
							printf("\nLas Contraseñas almacenadas son:\n");
							printf("\nContraseña de Administrador: %d\n",passAdmin);
							printf("\nContraseña de Proveedor: %d\n",passProveedor);
							printf("\n!!! Se han mostrado todas las contraseñas almacenadas !!! Presione Enter para Continuar... ");
							getchar(); getchar(); system("clear");
						break;
						
						case 2: // Opcion Cambiar contraseña de Administrador
							printf ("\nIngrese la CONTRASEÑA Actual de Administrador y presione Enter (6 Digitos) ~& ");
							scanf("%d",&i);
							if (i>=100000 && i<=999999){ // Contraseña actual ingresada dentro del rango permitido
								if(i==passAdmin){ // Contraseña actual de admin ingresada correctamente
									printf ("\nIngrese la NUEVA CONTRASEÑA de Administrador y presione Enter (6 Digitos) (Entre 100000 y 999999 ) ~& ");
									scanf("%d",&j);
									
									if (i>=100000 && i<=999999){ // Contraseña Nueva ingresada dentro del rango permitido
										printf ("\nRepita la NUEVA CONTRASEÑA de Administrador ingresada y presione Enter para confirmar (6 Digitos) ~& ");
										scanf("%d",&i);
										if(j==i){ // Las nuevas contraseñas coinciden y se actualiza exitosamente
											passAdmin=j;
											printf("\n!!! Contraseña de Administrador Actualizada con Exito !!! Presione Enter para Continuar... ");
										}else{ // Las nuevas contraseñas ingresadas no coinciden
											printf("\n!!! Las Contraseñas no Coinciden !!! Presione Enter para Intentar de Nuevo... ");
										}
									
									}else { //Contraseña Nueva ingresada Fuera de Rango
										printf("\n!!! La Nueva Contraseña Ingresada esta fuera de Rango !!! Presione Enter para Intentar de Nuevo... "); 
									}
									
								}else{ // Contraseña actual de admin Incorrecta
									printf("\n!!! Contraseña de Administrador Incorrecta !!! Presione Enter para Intentar de Nuevo... "); 
								}
							
							}else{ //Contraseña actual ingresada Fuera de Rango
								printf("\n!!! La Contraseña Ingresada esta fuera de Rango !!! Presione Enter para Intentar de Nuevo... "); 
							}
							getchar(); getchar(); system("clear");
						break;
						
						case 3: // Opcion Cambiar Contraseña de Proveedor
							printf ("\nIngrese la CONTRASEÑA Actual de Proveedor y presione Enter (6 Digitos) ~& ");
							scanf("%d",&i);
							if (i>=100000 && i<=999999){ // Contraseña actual ingresada dentro del rango permitido
								if(i==passProveedor){ // Contraseña actual de proveedor ingresada correctamente
									printf ("\nIngrese la NUEVA CONTRASEÑA de Proveedor y presione Enter (6 Digitos) (Entre 100000 y 999999 ) ~& ");
									scanf("%d",&j);
									
									if (i>=100000 && i<=999999){ // Contraseña Nueva ingresada dentro del rango permitido
										printf ("\nRepita la NUEVA CONTRASEÑA de Proveedor ingresada y presione Enter para confirmar (6 Digitos) ~& ");
										scanf("%d",&i);
										if(j==i){ // Las nuevas contraseñas coinciden y se actualiza exitosamente
											passProveedor=j;
											printf("\n!!! Contraseña de Proveedor Actualizada con Exito !!! Presione Enter para Continuar... ");
										}else{ // Las nuevas contraseñas ingresadas no coinciden
											printf("\n!!! Las Contraseñas no Coinciden !!! Presione Enter para Intentar de Nuevo... ");
										}
									
									}else { //Contraseña Nueva ingresada Fuera de Rango
										printf("\n!!! La Nueva Contraseña Ingresada esta fuera de Rango !!! Presione Enter para Intentar de Nuevo... "); 
									}
									
								}else{ // Contraseña actual de Proveedor Incorrecta
									printf("\n!!! Contraseña de Proveedor Incorrecta !!! Presione Enter para Intentar de Nuevo... "); 
								}
							
							}else{ //Contraseña actual ingresada Fuera de Rango
								printf("\n!!! La Contraseña Ingresada esta fuera de Rango !!! Presione Enter para Intentar de Nuevo... "); 
							}
							getchar(); getchar(); system("clear");
						break;
						
						case 4: // Opcion Reestablecer Contraseñas
							printf("\n ¿Desea Reestablecer TODAS las Contraseñas a su valor por defecto = (100000)? (SI=1 / NO=0)\n");
							printf ("Ingrese el numero de la opcion desea y presione Enter ~& ");
							scanf("%i",&i);//Se ingresa la opcion
			
							if(i=1){ // Se confirma reestablecer contraseñas
								passAdmin=100000;
								passProveedor=100000;
								printf("\n!!! Contraseñas Reestablecidas con Exito !!! Presione Enter para Continuar... ");
							}else { // Se cancela operacion de reestablecimiento de contraseñas
								printf("\n!!! Operacion de Reestablecimiento de Contraseñas Cancelada !!! Presione Enter para Continuar... ");
							}						
							getchar(); getchar(); system("clear");
						break;
						
						case 0: // Opcion Regresar al Menu Principal
							system("clear");
						break;
						
						default: // Opcion Invalida
							printf("\n!!! Ha Ingresado una Opcion Invalida !!! Presione Enter para Intentar de Nuevo... ");
							getchar(); getchar(); system("clear");
					} // Termina switch Submenu Gestionar Contraseñas
					
				}while(submenu!=0);
			break;
			
			case 0: // Mensaje al salir del programa

			if (!empty(catalogo)){ // Catalogo Tiene Productos  

					fileCatalogo = fopen("Catalogo.txt","w+"); // Se abre archivo para escritura
					if(fileCatalogo==NULL){
						printf("\n!!! No se ha podido acceder al archivo Catalogo.txt !!!\n ");
						exit(-1); 
					}

					for(i=0;i<catalogo->NE;i++){ // Se imprimen los elementos de la lista "Catalogo de Productos Disponibles"
						fprintf(fileCatalogo,"%s%d\n%d\n",get(i,catalogo).name, get(i,catalogo).cant, get(i,catalogo).precio);
					}
					fclose(fileCatalogo);
				}

				printf("\n!!! Hasta la proxima !!! Presione Enter para Continuar... "); getchar(); getchar(); system("clear"); main();
			break;

			default: printf("\n!!! Ha insertado una opcion invalida !!! Presione Enter para Intentar de nuevo...\n"); 	
		}
	}while(menu!=0); 
}

/*----------------------------------------------------------------------- CLIENTE ---------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

void menuCliente(){

// Se abre Fichero
	fileCatalogo = fopen("Catalogo.txt","r+");
	if(fileCatalogo==NULL){
		printf("\n!!! No se ha podido acceder al archivo Catalogo.txt !!!\n ");
		exit(-1); 
	}

	//Se precargan productos en la lista "Catalogo de Productos"
    size_t len = 0;
	pos=0;
    while (getline(&contents, &len, fileCatalogo) != -1){
        strcpy(auxList.name,contents);
		getline(&contents, &len, fileCatalogo);
		auxList.cant=atoi(contents);
		getline(&contents, &len, fileCatalogo);
		auxList.precio=atoi(contents);

		add (pos,auxList,catalogo);
		pos++;
		//printf("%s", contents);

    }
	pos=0;
	fclose(fileCatalogo);

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

				// SE IMPRIME TICKET DE COMPRA EN CASO DE TENER PRODUCTOS EN CARRITO

				if (!empty(cartClient)){ // Catalogo Tiene Productos  
					j=0;
					fileCatalogo = fopen("Ticket.txt","w+"); // Se abre archivo para escritura
					if(fileCatalogo==NULL){
						printf("\n!!! No se ha podido acceder al archivo Catalogo.txt !!!\n ");
						exit(-1); 
					}
						fprintf(fileCatalogo,"################################ ESCOMMERCE ########################################\n\n");
						fprintf(fileCatalogo,"----------------------------- Ticket de Compra -------------------------------------\n\n");

					for(i=0;i<cartClient->NE;i++){ // Se imprimen los elementos de la lista "Catalogo de Productos Disponibles"
						fprintf(fileCatalogo,"Producto : %sCantidad: %d\nPrecio Unitario : $%d\n\n",get(i,cartClient).name, get(i,cartClient).cant, get(i,cartClient).precio);
						j = j+ (get(i,cartClient).precio * get(i,cartClient).cant);
					}
					fprintf(fileCatalogo,"----------------------------------------------------------------------------------------\n\n");
					fprintf(fileCatalogo,"Total a Pagar = $%d\n",j);
					fprintf(fileCatalogo,"                         !!!! Gracias Por Su Preferencia !!!!");

					fclose(fileCatalogo);
				}
				// FIN TICKET DE COMPRA

			if (!empty(catalogo)){ // Catalogo Tiene Productos  

					fileCatalogo = fopen("Catalogo.txt","w+"); // Se abre archivo para escritura
					if(fileCatalogo==NULL){
						printf("\n!!! No se ha podido acceder al archivo Catalogo.txt !!!\n ");
						exit(-1); 
					}

					for(i=0;i<catalogo->NE;i++){ // Se imprimen los elementos de la lista "Catalogo de Productos Disponibles"
						fprintf(fileCatalogo,"%s%d\n%d\n",get(i,catalogo).name, get(i,catalogo).cant, get(i,catalogo).precio);
					}
					fclose(fileCatalogo);
				}

				printf("\n!!! Hasta la proxima !!! Presione Enter para Continuar... "); getchar(); getchar(); system("clear"); main();
			break;

			default: printf("\n!!! Ha insertado una opcion invalida !!! Presione Enter para Intentar de nuevo...\n"); getchar(); getchar(); system("clear");	
		}
	}while(menu!=0);
}
    
/*------------------------------------------------------------------------------- PROVEEDOR ------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/


void menuProveedor(){
// Se abre Fichero
	fileCatalogo = fopen("Catalogo.txt","r+");
	if(fileCatalogo==NULL){
		printf("\n!!! No se ha podido acceder al archivo Catalogo.txt !!!\n ");
		exit(-1); 
	}

	//Se precargan productos en la lista "Catalogo de Productos"
    size_t len = 0;
	pos=0;
    while (getline(&contents, &len, fileCatalogo) != -1){
        strcpy(auxList.name,contents);
		getline(&contents, &len, fileCatalogo);
		auxList.cant=atoi(contents);
		getline(&contents, &len, fileCatalogo);
		auxList.precio=atoi(contents);

		add (pos,auxList,catalogo);
		pos++;
		//printf("%s", contents);

    }
	pos=0;
	fclose(fileCatalogo);

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
										strcat(auxString,"\n");

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

			if (!empty(catalogo)){ // Catalogo Tiene Productos  

					fileCatalogo = fopen("Catalogo.txt","w+"); // Se abre archivo para escritura
					if(fileCatalogo==NULL){
						printf("\n!!! No se ha podido acceder al archivo Catalogo.txt !!!\n ");
						exit(-1); 
					}

					for(i=0;i<catalogo->NE;i++){ // Se imprimen los elementos de la lista "Catalogo de Productos Disponibles"
						fprintf(fileCatalogo,"%s%d\n%d\n",get(i,catalogo).name, get(i,catalogo).cant, get(i,catalogo).precio);
					}
				fclose(fileCatalogo);
				}

				printf("\n!!! Hasta la proxima !!! Presione Enter para Continuar... "); getchar(); getchar(); system("clear"); main();
			break;

			default: printf("\n!!! Ha insertado una opcion invalida !!! Presione Enter para Intentar de nuevo...\n"); getchar(); getchar(); system("clear");	
		}
	}while(menu!=0);

}

