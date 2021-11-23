#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>					
#include <sys/ipc.h>
#include <sys/shm.h>
#include <wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#define  TAM_MEMORIA 27                 //Tamaño de la memoria compartida en bytes

#include "carrito.c"


// Variables Globales
lista catalogo,cartClient;
carrito auxList;
unsigned menu,submenu,pos,i,j,condicional,auxCant,status,passProveedor;
char auxString[21];

void main()
{
	int fd, i, respCliente, buf[10], n;
    pid_t pid;

	int shm_id;								//Id de la memoria compartida
				key_t llave;							//Llave para poder acceder a la memoria compartida
				char *shm, *s;
				llave = 5678;							//Valor asignado a la llave, puede estar entre 1 y 65,536
				shm_id = shmget (llave, TAM_MEMORIA, 0666);			//Obtenemos memoria compartida con la llave del servidor
				if (shm_id < 0)
				{
					perror ("Error al obtener la memoria compartida: shmget");
					exit(-1);
				}
				shm = shmat (shm_id, NULL, 0);			//Enlace de memoria compartida
				if (shm == (char *)-1)
				{
					perror ("Error al enlazar la memoria compartida: shmat");
					exit(-1);
				}

	// Se inicializan Contraseñas por default
	passProveedor = 100000;

	printf("\nIngrese la contraseña de Proveedor (Si es ejecucion por primera vez, ingrese 100000 ) ~& ");
				scanf("%d",&i);
				if(i>=100000 && i<=999999){ // Contraseña Ingresada dentro del rango permitido
					if (i == passProveedor){
					}else{
						printf("\n!!! Contraseña Incorrecta !!! Presione Enter para Intentar de Nuevo... "); 
						getchar(); getchar(); system("clear");
					}
				}else{ // Contraseña ingresada fuera del rango permitido
					printf("\n!!! Contraseña Ingresada fuera del rango permitido !!! Presione Enter para Intentar de Nuevo... ");
					getchar(); getchar(); system("clear");
				}

	/*// Proveedor avisa a Servidor que se ha conectado
    mkfifo("/tmp/mainTienda", 0666);
    fd = open("/tmp/mainTienda", O_WRONLY);
    write(fd, &buf[0], 1);
    close(fd);

	if(n==1){ // Proveedor se ha conectado
            pid = fork();
            if (pid == -1){
                perror("\nError al crear el proceso\n");
                exit(-1);
            }
        if (pid == 0){ // Es el Proceso Hijo
            //printf("\nSoy el Hijo = %d y mi Padre es = %d\n",getpid(),getppid());      
            printf("!!! Cliente Conectado !!!");
            sleep(1);*/

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


system("clear");

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
				printf("\n!!! Hasta la proxima !!! Presione Enter para Continuar... "); getchar(); getchar(); system("clear");
			break;

			default: printf("\n!!! Ha insertado una opcion invalida !!! Presione Enter para Intentar de nuevo...\n"); getchar(); getchar(); system("clear");	
		}
	}while(menu!=0);
  }
	
