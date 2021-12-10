#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>					
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wait.h>
#include "../include/cliente.h"
#include "../include/systemv.h"

// Prototipos
void atender_cliente(void *argumentos);
void menuAdmin();
void menuCliente();

// Variables Globales
lista cuentaCliente;
cuenta auxList;
unsigned int server,menu,submenu,pos,i,j,n,fd,auxNipSaldo,passAdmin,movimientos;
char auxString[31], *contents = NULL, auxNumCuenta1[5], auxNumCuenta2[5],buffer[9];
unsigned status;
FILE* fileCuentaCliente;

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
	crearlista(&cuentaCliente); // cuentaCliente de Productos de la Tienda

	srand(time(NULL));	
	passAdmin=123456;

	do{ // Inicia SERVIDOR
		system("clear");
		printf("*************** Practica 6 ***************\n\n");
		printf ("********************** INICIO *************************\n\n"); 
		printf("!!! Bienvenido al Cajero Automatico !!!\n\n");
		printf("1) Acceder como Administrador \n2) Realizar Conexion con un Cliente \n0) Salir de la Tienda\n\n");	
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
                    printf("Esperando Tarjeta de Cliente...\n");
                    down(sem_servidor);
                    printf("Tarjeta Detectada, creando hilo...\n");

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

			case 0:
				printf("!!! Gracias por Utilizar el Cajero Automatico !!! Hasta la Proxima \n\n");
				free(contents);
				exit(0);
			break;
		
			default:
				printf("\n!!! Ha insertado una opcion invalida !!! Presione Enter para Intentar de Nuevo..."); 
				getchar(); getchar(); system("clear");
			break;
		}

	} while (server!=0);
	
	liberarlista (&cuentaCliente);
	return 0;
}

// **************************************************************************** ADMINISTRADOR *******************************************************************************************
//***************************************************************************************************************************************************************************************
void menuAdmin(){

	// Se abre Fichero
	fileCuentaCliente = fopen("Cuentas.txt","r+");
	if(fileCuentaCliente==NULL){
		printf("\n!!! No se ha podido acceder al archivo Cuentas.txt !!!\n ");
		exit(-1); 
	}

	//Se cargan cuentas de clientes desde el fichero
    size_t len = 0;
	pos=0;
    while (getline(&contents, &len, fileCuentaCliente) != -1){
        auxList.numCuenta=atol(contents);
		getline(&contents, &len, fileCuentaCliente);
		strcpy(auxList.name,contents);
		getline(&contents, &len, fileCuentaCliente);
		auxList.nip=atoi(contents);
		getline(&contents, &len, fileCuentaCliente);
		auxList.saldo=atoi(contents);

		add (pos,auxList,cuentaCliente);
		pos++;
    }
	pos=0;
	fclose(fileCuentaCliente);

	do{// Comienza Menu Principal del Programa
		system("clear");
		printf("\t*************** Practica 6 ***************\n\n");
		printf ("********************** MENU PRINCIPAL - Administrador *************************\n\n");
		printf("1) Mostrar Cuentas Registradas \n2) Gestionar Cuentas\n");
		printf("3) Gestionar Contraseña/Nip's \n0) Salir de la Cuenta de Adminitrador\n\n");	
		printf("Ingrese el NUMERO de la opcion deseada y presione ENTER ~& ");
		scanf("%i",&menu);
	
		switch (menu){ // Operaciones para el MENU PRINCIPAL
			case 1: // 1) Mostrar Cuentas
				printf ("\n---- Cuentas Registradas ----\n\n");    
				for(i=0;i<cuentaCliente->NE;i++){ // Se imprimen las cuentas registradas
	  				printf("(%d) Numero de Cuenta: %lu \n",i,get(i,cuentaCliente).numCuenta);
					printf("Titular: %s\n",get(i,cuentaCliente).name);
					printf("Saldo: $%d Pesos.\n\n",get(i,cuentaCliente).saldo);   
				}
				printf("!!! Se han mostrado todas las cuentas registradas !!! Presione Enter para Continuar... ");	  
				getchar(); getchar(); system("clear");
			break;
			
			case 2: // 2) GESTIONAR Cuentas
				do{ // Inicia Submenu Gestionar Cuentas
					system("clear");
					printf("\t*************** Proyecto Tienda Online ***************\n\n");
					printf ("********************** SUBMENU GESTIONAR CUENTAS - Admin *************************\n\n"); 
					printf("1) Ver Cuentas Registradas \n2) Registrar Nueva Cuenta \n");
					printf("3) Borrar una Cuenta \n4) Limpiar Base de Datos \n0) Regresar al Menu Principal\n\n"); // Opciones 	
					printf("Ingrese el NUMERO de la operacion deseada y presione ENTER ~& ");
					scanf("%i",&submenu);
									
					switch(submenu){
						case 1: // Opcion Ver Cuentas Registradas
							if (!empty(cuentaCliente)){ // Verifica si existen cuentas
								printf ("\n---- Cuentas Registradas ----\n\n");    
							for(i=0;i<cuentaCliente->NE;i++){ // Se imprimen las cuentas registradas
								printf("(%d) Numero de Cuenta: %lu\n",i,get(i,cuentaCliente).numCuenta);
								printf("Titular: %s\n",get(i,cuentaCliente).name);
								printf("Saldo: $%d Pesos.\n\n",get(i,cuentaCliente).saldo);   
							}
							printf("!!! Se han mostrado todas las cuentas registradas !!! Presione Enter para Continuar... ");	  
							}else{ // no hay cuentas registradas
								printf("\n!!! No existen Cuentas Registradas !!! Presione Enter para continuar..."); 	
							}	
							getchar(); getchar(); system("clear");
						break;
											
						case 2: // Opcion Registrar Nueva Cuenta 
									// Genera Numero de Cuenta Aleatoria
									i=rand()%10000 + 1000;
									itoa(i,auxNumCuenta1,10);
									j=rand()%10000 + 1000;
									itoa(j,auxNumCuenta2,10);

									strcpy(buffer,strcat(auxNumCuenta1,auxNumCuenta2));
									auxList.numCuenta=atol(buffer);

									printf("\nIngrese el Nombre del Nuevo Titular y presione ENTER. (Max. 30 Caracteres) ~& ");
									scanf("%30[^\n]s",auxString); //Se ingresa el nombre del Nuevo Titular
									getchar();
									strcat(auxString,"\n");

									auxList.nip=rand()%10000 + 1234;
									auxList.saldo=0;
									
									add (0,auxList,cuentaCliente);

									// Se muestra los datos de la nueva cuenta
									printf ("\n---- Datos de la Cuenta Nueva ----\n\n");    									
									printf("Numero de Cuenta: %lu\n",get(i,cuentaCliente).numCuenta);
									printf("Titular: %s\n",get(i,cuentaCliente).name);
									printf("NIP asignado: %d.\n\n",get(i,cuentaCliente).nip);
									printf("Saldo: $%d Pesos.\n\n",get(i,cuentaCliente).saldo);   
									printf ("\n-------------------------------------------\n\n");

									i=0;
									j=0;
									printf("\n!!! Cuenta Registrada Exitosamente !!! Presione Enter para Continuar... "); 
									getchar(); getchar(); system("clear");
						break;

						case 3: // Opcion Borrar una Cuenta
							if (!empty(cuentaCliente)){ // Se verifica si existen cuentas
								printf ("\n---- Cuentas Registradas ----\n\n");    
								for(i=0;i<cuentaCliente->NE;i++){ // Se imprimen las cuentas registradas
									printf("(%d) Numero de Cuenta: %lu\n",i,get(i,cuentaCliente).numCuenta);
									printf("Titular: %s\n",get(i,cuentaCliente).name);
									printf("Saldo: $%d Pesos.\n\n",get(i,cuentaCliente).saldo);   
								}
								printf ("\n-------------------------------------------\n\n");
								
								printf("\nSeleccione el INCISO \"(#)\" correspondiente a la cuenta que desea Borrar (0 - %d) ~& \n",cuentaCliente->NE);
								scanf("%i",&pos);//Se ingresa la posicion
								if (pos>=0 && pos<cuentaCliente->NE){
									borrar (pos,cuentaCliente); // Se borra cuenta
									printf("\n!!! Cuenta Eliminada !!! Presione Enter para Continuar... ");
								} else { // Se ingresa una posicion invalida
									printf("\n!!! Inciso ingresado es invalido !!! Presione Enter para Intentar de Nuevo... ");
								}												
							}else{ // No hay Cuentas Registradas
								printf("\n!!! No hay Cuentas Registradas !!! Presione Enter para Continuar... "); 
							}
							getchar(); getchar(); system("clear");
						break;
						
						case 4: // Opcion Limpiar Base de Datos
							if (!empty(cuentaCliente)){ // Se verifica si existen cuentas
								printf ("\n---- Cuentas Registradas ----\n\n");    
								for(i=0;i<cuentaCliente->NE;i++){ // Se imprimen las cuentas registradas
									printf("(%d) Numero de Cuenta: %lu\n",i,get(i,cuentaCliente).numCuenta);
									printf("Titular: %s\n",get(i,cuentaCliente).name);
									printf("Saldo: $%d Pesos.\n\n",get(i,cuentaCliente).saldo);   
								}
								printf ("\n-------------------------------------------\n\n");
								
								printf ("Ingrese la CONTRASEÑA de Administrador y presione Enter (6 Digitos) ~& ");
								scanf("%i",&i);
								if (i>=100000 && i<=999999){ // Contraseña ingresada dentro del rango permitido
									if(i==passAdmin){ // Contraseña de admin ingresada correctamente
										printf("\n ¿Desea Borrar TODO el contenido de la Base de Datos? (SI=1 / NO=0)\n");
										printf ("Ingrese el numero de la opcion desea y presione Enter ~& ");
										scanf("%i",&i);//Se ingresa la opcion
										
										if(i==1){ // Se confirma la limpieza de la Base de Datos
											vaciarlista(cuentaCliente);	
											printf("\n!!! Base de Datos vaciado exitosamente !!! Presione Enter para Continuar... "); 
										}else{ // Operacion de Limpieza total cancelada por admin
											printf("\n!!! Operacion de Limpieza Cancelada !!! Presione Enter para Continuar... "); 
										}
										
									}else{ // Contraseña de admin Incorrecta
										printf("\n!!! Contraseña de Administrador Incorrecta !!! Presione Enter para Intentar de Nuevo... "); 
									}
								
								}else { //Contraseña ingresada Fuera de Rango
									printf("\n!!! La Contraseña Ingresada esta fuera de Rango !!! Presione Enter para Intentar de Nuevo... "); 
								}												
							}else{ // No hay Cuentas Registradas
								printf("\n!!! No hay Cuentas Registradas !!! Presione Enter para Continuar... "); 
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
					printf("\t*************** Practica 6 ***************\n\n");
					printf ("********************** SUBMENU GESTIONAR CONTRASEÑAS - Admin *************************\n\n"); 
					printf("1) Cambiar Contraseña de Administrador \n2) Reestablecer NIP de Cuenta \n0) Regresar al Menu Principal\n\n"); // Opciones 	
					printf("Ingrese el NUMERO de la operacion deseada y presione ENTER ~& ");
					scanf("%i",&submenu);
					
					switch (submenu){
						case 1: // Opcion Cambiar contraseña de Administrador
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
						
						case 2: // Opcion Reestablecer NIP de Cuenta
							if (!empty(cuentaCliente)){
								printf ("\nIngrese la CONTRASEÑA Actual de Administrador y presione Enter (6 Digitos) ~& ");
								scanf("%d",&i);
								if (i>=100000 && i<=999999){ // Contraseña actual ingresada dentro del rango permitido
									if(i==passAdmin){ // Contrasena correcta
										printf ("\n---- Cuentas Registradas ----\n\n");    
										for(i=0;i<cuentaCliente->NE;i++){ // Se imprimen las cuentas registradas
											printf("(%d) Numero de Cuenta: %lu\n",i,get(i,cuentaCliente).numCuenta);
											printf("Titular: %s\n",get(i,cuentaCliente).name);
											printf("Saldo: $%d Pesos.\n\n",get(i,cuentaCliente).saldo);   
										}
										printf ("\n-------------------------------------------\n\n");
										printf("Ingrese el INCISO correspondiente a la cuenta que desea reestablecer su NIP y presione ENTER (0 - %d) ~& ",cuentaCliente->NE);
										scanf("%i",&i);

										if (i>=0 && i<=cuentaCliente->NE){ // Inciso ingresado dentro del rango y valido
															// Se obtiene datos de Cuenta elegida
															auxList.numCuenta=get(i,cuentaCliente).numCuenta;
															strcpy(auxList.name,get(i,cuentaCliente).name);
															auxList.nip=1234; // Se reestablece nip 
															auxList.saldo=get(i,cuentaCliente).saldo;
															
															borrar (i,cuentaCliente);
															add (i,auxList,cuentaCliente);

										} else{ // Inciso ingresado fuera de rango
											printf("\n!!! Ha insertado una inciso invalido !!! Presione Enter para Intentar de nuevo... "); 
										}

									}else{ // Contraseña actual de admin Incorrecta
										printf("\n!!! Contraseña de Administrador Incorrecta !!! Presione Enter para Intentar de Nuevo... "); 
									}
								
								}else{ //Contraseña actual ingresada Fuera de Rango
									printf("\n!!! La Contraseña Ingresada esta fuera de Rango !!! Presione Enter para Intentar de Nuevo... "); 
								}

							}else{// No hay Clientes Registrados
								printf("\n!!! No hay Cuentas Registradas !!! Presione Enter para Intentar de Nuevo... "); 
							}
							getchar(); getchar(); system("clear");
						break;
						
						case 0: // Opcion Regresar al Menu Principal
							system("clear");
						break;
						
						default: // Opcion Invalida
							printf("\n!!! Ha Ingresado una Opcion Invalida !!! Presione Enter para Intentar de Nuevo... ");
							getchar(); getchar(); system("clear");
					} // Termina switch Submenu Gestionar Contraseña/NIP
					
				}while(submenu!=0);
			break;
			
			case 0: // Mensaje al salir del programa
			if (!empty(cuentaCliente)){ // Hay Cuentas Registradas
					fileCuentaCliente = fopen("Cuentas.txt","w+"); // Se abre archivo para escritura
					if(fileCuentaCliente==NULL){
						printf("\n!!! No se ha podido acceder al archivo Cuentas.txt !!!\n ");
						exit(-1); 
					}

					for(i=0;i<cuentaCliente->NE;i++){ // Se imprimen los elementos de la lista "cuentaCliente de Productos Disponibles"
						fprintf(fileCuentaCliente,"%lu\n%s%d\n%d\n",get(i,cuentaCliente).numCuenta,get(i,cuentaCliente).name, get(i,cuentaCliente).nip, get(i,cuentaCliente).saldo);
					}
					fclose(fileCuentaCliente);
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
	fileCuentaCliente = fopen("Cuentas.txt","r+");
	if(fileCuentaCliente==NULL){
		printf("\n!!! No se ha podido acceder al archivo Cuentas.txt !!!\n ");
		exit(-1); 
	}

	//Se cargan cuentas de clientes desde el fichero
    size_t len = 0;
	pos=0;
    while (getline(&contents, &len, fileCuentaCliente) != -1){
        auxList.numCuenta=atol(contents);
		getline(&contents, &len, fileCuentaCliente);
		strcpy(auxList.name,contents);
		getline(&contents, &len, fileCuentaCliente);
		auxList.nip=atoi(contents);
		getline(&contents, &len, fileCuentaCliente);
		auxList.saldo=atoi(contents);

		add (pos,auxList,cuentaCliente);
		pos++;
    }
	pos=0;
	fclose(fileCuentaCliente);

	// SE VERIFICA SI EXISTEN CUENTAS
	// SE PREGUNTA POR EL NIP, SI FALLA ENTONCES SE REGRESA AL MENU INICIO
		// UNA VEZ INGRESADO NIP, SE RECORRE (con j) LISTA DE CUENTAS PARA VER COINCIDENCIAS Y SABER QUE CUENTA(ELEMENTO) SE USARA (ESE ELEMenTO SERA i)
		// SI NO HAY COINCIDENCIAS, SE MANDA NIP INCORRECTA
	

do{// Comienza Menu Principal del Cliente
		printf("\t*************** Practica 6 ***************\n\n");
		printf ("********************** MENU PRINCIPAL - Cliente *************************\n\n"); 
		printf("1) Mostrar Saldo\n2) Realizar Deposito\n3) Realizar Retiro\n4) Realizar Transferencia\n5) Cambiar NIP \n0) Salir de la Cuenta Cliente\n\n");	
		printf("Ingrese el NUMERO de la opcion deseada y presione ENTER ~& ");
		scanf("%i",&menu);
	
		switch (menu){ // Operaciones para el MENU PRINCIPAL
			case 1: // 1) Mostrar Saldo
				printf ("\nSu Saldo Actual es de $%d\n",get(i,cuentaCliente).saldo);    
				printf("\n!!! Fin de la Consulta de Saldo !!! Presione Enter para Continuar...");
				getchar(); getchar(); system("clear");
			break;

			case 2: // 2) Realizar Deposito
				printf ("\nIngrese el MONTO que desea Depositar en su Cuenta y presione Enter (Max. $10,000) ~& ");
				scanf("%d",&j);

				if(j>=0 && j<=10000){ // Se ingresa monto dentro de rango permitido

					// Se obtiene datos de Cuenta elegida
					auxList.numCuenta=get(i,cuentaCliente).numCuenta;
					strcpy(auxList.name,get(i,cuentaCliente).name);
					auxList.nip=get(i,cuentaCliente).nip;  
					auxList.saldo=get(i,cuentaCliente).saldo + j; // Se actualiza saldo
					
					borrar (i,cuentaCliente);
					add (i,auxList,cuentaCliente);

					printf ("\nSu NUEVO Saldo es de $%d\n",get(i,cuentaCliente).saldo);    
					printf("\n!!! Fin de la Consulta de Saldo !!! Presione Enter para Continuar...");
				}else{ // Monto ingresado fuera de Rango
					printf("\n!!! Ha insertado un Monto invalido !!! Presione Enter para Intentar de nuevo..."); 	
				}
				getchar(); getchar(); system("clear");
			break;

			case 3:	// 3) Realizar Retiro
			break;

			case 4: // 4) Realizar Transferencia
			break;

			case 5: // 5) Cambiar NIP
				j=0;
				printf ("\nIngrese su NIP actual y presione Enter (4 Digitos) ~& ");
				scanf("%d",&j);

				if(j>=0 && j<=10000){ // Se ingresa monto dentro de rango permitido

					// Se obtiene datos de Cuenta elegida
					auxList.numCuenta=get(i,cuentaCliente).numCuenta;
					strcpy(auxList.name,get(i,cuentaCliente).name);
					auxList.nip=get(i,cuentaCliente).nip;  
					auxList.saldo=get(i,cuentaCliente).saldo + j; // Se actualiza saldo
					
					borrar (i,cuentaCliente);
					add (i,auxList,cuentaCliente);

					printf ("\nSu NUEVO NIP es %d !! Recuerdelo y No lo Comparta con Nadie !!\n",get(i,cuentaCliente).nip);    
					printf("\n!!! Fin de la Consulta de Saldo !!! Presione Enter para Continuar...");
				}else{ // NIP Actual Incorrecta
					printf("\n!!! NIP incorrecto  !!! Presione Enter para Intentar de nuevo..."); 	
				}
				getchar(); getchar(); system("clear");
			break;

			case 0: // Mensaje al salir del programa

				// SE IMPRIME TICKET DE MOVIMIENTOS SI SE REALIZO MOVIMIENTO
				if (movimientos==1){   
					fileCuentaCliente = fopen("Ticket.txt","w+"); // Se abre archivo para escritura
					if(fileCuentaCliente==NULL){
						printf("\n!!! No se ha podido acceder al archivo cuentaCliente.txt !!!\n ");
						exit(-1); 
					}
						fprintf(fileCuentaCliente,"################################ Banco ########################################\n\n");
						fprintf(fileCuentaCliente,"----------------------------- Comprobante -------------------------------------\n\n");

						 // Se imprimen los movimientos realizados

						fprintf(fileCuentaCliente,"Producto : %sCantidad: %lu\nPrecio Unitario : $%d\n\n",get(i,cuentaCliente).name, get(i,cuentaCliente).numCuenta, get(i,cuentaCliente).saldo);
					fprintf(fileCuentaCliente,"----------------------------------------------------------------------------------------\n\n");
					fprintf(fileCuentaCliente,"                         !!!! Gracias Por Su Preferencia !!!!");

					fclose(fileCuentaCliente);
				}	// FIN TICKET DE COMPRA

				// Se actualiza Base de Datos
				fileCuentaCliente = fopen("Cuentas.txt","w+"); // Se abre archivo para escritura
				if(fileCuentaCliente==NULL){
					printf("\n!!! No se ha podido acceder al archivo Cuentas.txt !!!\n ");
					exit(-1); 
				}

				for(i=0;i<cuentaCliente->NE;i++){ // Se imprimen las cuentas a base de datos
				fprintf(fileCuentaCliente,"%lu\n%s%d\n%d\n",get(i,cuentaCliente).numCuenta,get(i,cuentaCliente).name, get(i,cuentaCliente).nip, get(i,cuentaCliente).saldo);
				}
				fclose(fileCuentaCliente);

				printf("\n!!! Hasta la proxima !!! Presione Enter para Continuar... "); getchar(); getchar(); system("clear"); main();
			break;

			default: printf("\n!!! Ha insertado una opcion invalida !!! Presione Enter para Intentar de nuevo...\n"); getchar(); getchar(); system("clear");	
		}
	}while(menu!=0);
}