#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include "include/systemv.h"

/* -------------------- Declaración de las funciones de los hilos -------------------- */
void administrador(void *);
void proveedor(void *);
void cliente(void *);

int main(void)
{
    int contador;
    int sem_servidor;
    int sem_proceso;
    int tipo_proceso;
    int llaves_conexion[20][2];
    int *comunicacion = NULL;

    /* ------------------------- Memorias compartidas ------------------------- */

    /** 
     * Esta memoria compartida es para que el servidor y el proceso que se está
     * conectando se puedan comunicar.
     * En el índice 0 de la memoria compartida el proceso indica su tipo, un proceso
     * puede ser de 3 tipos:
     *     1 - Administrador
     *     2 - Proveedor
     *     3 - Cliente
     * En los índices 1 y 2 de la memoria coompartida el servidor le pasa al proceso
     * sus claves para que se pueda comunicar con el hilo que le fue asignado.
     */
    comunicacion = (int *)shm(sizeof(int)*3, "shm_comunicacion", 'v');

    /* ------------------------------ Semáforos ------------------------------- */

    /* Semáforo del servidor */
    sem_servidor = sem(0, "sem_servidor", 'w');

    /* Semáforo del cliente */
    sem_proceso = sem(0, "sem_proceso", 'x');

    /* --------------------------- Configuraciones ---------------------------- */

    /* Llenado del arreglo con las llaves de conexión para la memoria compartida con hilos */
    for (contador = 0; contador < 20; contador++) {
        llaves_conexion[contador][0] = contador + 97;
        llaves_conexion[contador][1] = true;
    }

    /* Hilos para atender a los procesos */
    pthread_attr_t atributos;
	pthread_t hilos[20];
	pthread_attr_init (&atributos);
	pthread_attr_setdetachstate (&atributos, PTHREAD_CREATE_DETACHED);

    /* -------------------------- Proceso principal --------------------------- */

    while (1) {
        down(sem_servidor);

        tipo_proceso = comunicacion[0]; // Tipo de proceso obtenido

        for (contador = 0; contador < 20; contador++) {
            if (llaves_conexion[contador][1]) {
                comunicacion[1] = llaves_conexion[contador][0];
                comunicacion[2] = contador+200;
                llaves_conexion[contador][1] = false;

                switch(tipo_proceso) {
                    case 1:
                        pthread_create(&hilos[contador], &atributos, (void *)administrador, (void *)comunicacion);
                        break;
                    case 2:
                        pthread_create(&hilos[contador], &atributos, (void *)proveedor, (void *)comunicacion);
                        break;
                    case 3:
                        pthread_create(&hilos[contador], &atributos, (void *)cliente, (void *)comunicacion);
                        break;
                    default:
                        printf("[%d] Tipo de proceso desconocido\n", getpid());
                        break;
                }

                break;
            }
        }

        up(sem_proceso);
        down(sem_servidor);

        comunicacion[0] = '@';
        comunicacion[1] = '@';
        comunicacion[2] = '@';
    }

    return 0;
}

/* -------------------- Definición de las funciones de los hilos --------------------- */

void administrador(void *parametros)
{
    int *argumentos = (int *)parametros;
    int clave, archivo;

    clave = argumentos[1];
    archivo = argumentos[2];

    printf("[Hilo %ld] Hilo administrador creado con clave: %c, archivo: %d\n", pthread_self(), clave, archivo);

    pthread_exit(NULL);
}

void proveedor(void *parametros)
{
    int *argumentos = (int *)parametros;
    int clave, archivo;

    clave = argumentos[1];
    archivo = argumentos[2];

    printf("[Hilo %ld] Hilo proveedor creado con clave: %c, archivo: %d\n", pthread_self(), clave, archivo);

    pthread_exit(NULL);
}

void cliente(void *parametros)
{
    int *argumentos = (int *)parametros;
    int clave, archivo;

    clave = argumentos[1];
    archivo = argumentos[2];

    printf("[Hilo %ld] Hilo cliente creado con clave: %c, archivo: %d\n", pthread_self(), clave, archivo);

    pthread_exit(NULL);
}
