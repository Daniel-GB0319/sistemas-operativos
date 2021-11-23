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
/*void menuAdmin();
void menuCliente();
void menuProveedor();*/

// Variables Globales
lista catalogo,cartClient;
carrito auxList;
unsigned int server,menu,submenu,pos,i,j,n,fd,condicional,totalClient,auxCant,passAdmin,passProveedor;
char auxString[21];
unsigned status;

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

                    //menuCliente();
                   
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
                    
                    //menuProveedor();
            }
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
