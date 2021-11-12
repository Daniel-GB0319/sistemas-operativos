#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>					//Servidor de la memoria compartida
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wait.h>
#include "carrito.c"
#define  TAM_MEMORIA 27                 //Tamaño de la memoria compartida en bytes

// Prototipos
void menuAdmin();

// Variables Globales
lista catalogo;
carrito auxList;
unsigned int server,menu,submenu,pos,i,j,n,fd,condicional,totalClient,auxCant,passAdmin,passProveedor;
char auxString[21];

int main(int argc, char *argv[]) {

	int shm_id;							//Identificador de la memoria compartida
	key_t llave;						//Llave para acceder a la memoria compartida
	char *shm, *s, c;
	llave = 5678;						//Mismo valor de llave que tiene el cliente
	shm_id = shmget (llave, TAM_MEMORIA, IPC_CREAT|0666);		//Creamos la memoria compartida
	if (shm_id < 0){
		perror ("Error al crear la memoria compartida: shmget");
		exit(-1);
	}
	shm = shmat (shm_id, NULL, 0);
	if (shm == (char *)-1){
		perror ("Error al enlazar la memoria compartida: shmat");
		exit(-1);
	}else{
		printf("Servidor Inicializandose! \n");
		sleep(2);
	}
	
	//Se crean las listas a utilizar
	crearlista(&catalogo); // Catalogo de Productos de la Tienda
	
	//Se precargan productos en la lista "Catalogo de Productos"
	strcpy(auxList.name,"Cuaderno");
	auxList.cant=3;
	auxList.precio=10;
	add(0,auxList,catalogo); 

	strcpy(auxList.name,"Boligrafo");
	auxList.cant=3;
	auxList.precio=5;
	add(1,auxList,catalogo);
	
	strcpy(auxList.name,"Libro");
	auxList.cant=3;
	auxList.precio=20;
	add(2,auxList,catalogo);
	
	strcpy(auxList.name,"Mochila");
	auxList.cant=3;
	auxList.precio=10;
	add(3,auxList,catalogo);

	// Se inicializan Contraseñas por default
	passAdmin = 100000;
	passProveedor = 100000;

	do{ // Inicia SERVIDOR
		system("clear");
		printf("\t*************** Proyecto Tienda Online ***************\n\n");
		printf ("********************** INICIO *************************\n\n"); 
		printf("!!! Bienvenido a Escommerce !!!\n\n");
		printf("1) Acceder como Admin \n2) Realizar Conexion con un Cliente/Proveedor \n0) Salir de la Cuenta Cliente\n\n");	
		printf("Ingrese el NUMERO de la opcion deseada y presione ENTER ~& ");
		scanf("%i",&server);

		switch (server){
			case 1: // MENU ADMIN
				printf("\nIngrese la contraseña de Administrador (Si es ejecucion por primera vez, ingrese 100000 ) ~& ");
				scanf("%d",&i);
				if(i>=100000 && i<=999999){ // Contraseña Ingresada dentro del rango permitido
					if (i == passAdmin){
					menuAdmin();
					}else{
						printf("\n!!! Contraseña Incorrecta !!! Presione Enter para Intentar de Nuevo... "); 
						getchar(); getchar(); system("clear");
					}
				}else{ // Contraseña ingresada fuera del rango permitido
					printf("\n!!! Contraseña Ingresada fuera del rango permitido !!! Presione Enter para Intentar de Nuevo... ");
					getchar(); getchar(); system("clear");
				}
			break;

			case 2: // Opcion Conexion
				printf("\nEsperando Conexion... ");
				
				do{
					fd = open("/tmp/mainTienda", O_RDONLY);
        			n = read(fd, i, sizeof(int));
					close(fd);

					if(n==1){ // Se conecta un Cliente
						fd = open("/tmp/mainTienda", O_WRONLY);
						write(fd, catalogo, sizeof(catalogo)); // Se envia el Catalogo
						close(fd);

						fd = open("/tmp/mainTienda", O_RDONLY); // Se recibe Catalogo actualizado
        				catalogo = read(fd, catalogo, sizeof(catalogo));
						close(fd);
					}

					if(n==2){ // Se conecta un Proveedor
						fd = open("/tmp/mainTienda", O_WRONLY);
						write(fd, catalogo, sizeof(catalogo)); // Se envia el Catalogo
						close(fd);

						fd = open("/tmp/mainTienda", O_WRONLY);
						write(fd, passProveedor, sizeof(unsigned int)); // Se envia el Catalogo
						close(fd);

						fd = open("/tmp/mainTienda", O_RDONLY); // Se recibe Catalogo actualizado
        				catalogo = read(fd, catalogo, sizeof(catalogo));
						close(fd);
					}
				}while (n!=1 || n!=2);
				
			break;

			case 0:
				printf("!!! Gracias por Utilizar Escommerce !!! Hasta la Proxima \n\n");
			break;
		
			default:
				printf("\n!!! Ha insertado una opcion invalida !!! Presione Enter para Intentar de Nuevo..."); 
				getchar(); getchar(); system("clear");
			break;
		}

	} while (server!=0);
	
	liberarlista (&catalogo);
	return 0;
}


// ############################################################ FUNCIONES #######################################################################


// **************************************************************************** ADMINISTRADOR *******************************************************************************************
//***************************************************************************************************************************************************************************************
void menuAdmin(){
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
				printf("\n!!! Hasta la proxima !!! Presione Enter para Continuar... "); getchar(); getchar(); system("clear");
			break;

			default: printf("\n!!! Ha insertado una opcion invalida !!! Presione Enter para Intentar de nuevo...\n"); 	
		}
	}while(menu!=0); 
}

