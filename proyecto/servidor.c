#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include "../include/systemv.h"

void atender_cliente(void *argumentos);

int main(void)
{
    int contador;
    int sem_servidor;
    int sem_cliente;
    int llaves_conexion[20][2];
    char numero_asiento[3]; // Para convertir el número de asiento a cadena
    char archivo_asiento[15]; // Para alamcenar el nombre del archivo para la llave de cada asiento
    int *asientos = NULL;
    int *parametros_cliente = NULL;

    /* ------------------------- Memorias compartidas ------------------------- */

    /* Arreglo de asientos */
    asientos = (int *)shm(sizeof(int)*20, "shm_asientos", 'u');

    /** 
     * Memoria compartida para asignar llaves de comunicación al cliente
     * nuevo para que pueda comunicarse con el hilo que le es asignado
     */
    parametros_cliente = (int *)shm(sizeof(int)*2, "shm_parametros_cliente", 'v');

    /* ------------------------------ Semáforos ------------------------------- */

    /* Semáforo del servidor */
    sem_servidor = sem(0, "sem_servidor", 'w');

    /* Semáforo del cliente */
    sem_cliente = sem(0, "sem_cliente", 'x');

    /* Llenado del arreglo de asientos */
    for (contador = 0; contador < 20; contador++) {
        /* Creación del nombre del archivo de la clave del asiento en el índice @contador */
        sprintf(numero_asiento, "%d", contador+1);
        strcpy(archivo_asiento, "sem_asiento_");
        strcat(archivo_asiento, numero_asiento);

        /**
         * Creación del semáforo para el asiento en el índice @contador.
         * 
         * Ya que los asientos son administrados por medio de un arreglo de enteros,
         * podemos utilizar cada elemento del arreglo como semáforo del asiento
         * correspondiente en el índice @contador.
         */
        asientos[contador] = sem(1, archivo_asiento, contador+100);
    }

    /* --------------------------- Configuraciones ---------------------------- */

    /* Llenado del arreglo con las llaves de conexión para la memoria compartida con hilos */
    for (contador = 0; contador < 20; contador++) {
        llaves_conexion[contador][0] = contador + 97;
        llaves_conexion[contador][1] = true;
    }

    /* Hilos para atender a los clientes */
    pthread_attr_t atributos;
	pthread_t hilos[20];
	pthread_attr_init (&atributos);
	pthread_attr_setdetachstate (&atributos, PTHREAD_CREATE_DETACHED);

    while (1) {
        printf("A la espera de un cliente...\n");
        down(sem_servidor);
        printf("Nuevo cliente conectado, creando hilo...\n");

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
    }

    return 0;
}
