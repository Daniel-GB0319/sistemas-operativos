#include <stdio.h>
#include "../include/systemv.h"

int main(void)
{
    int contador;
    int sem_servidor;
    int sem_cliente;
    char numero_asiento[3]; // Para convertir el número de asiento a cadena
    char archivo_asiento[15]; // Para alamcenar el nombre del archivo para la llave de cada asiento
    int *asientos = NULL;
    int *claves_comunicacion_hilo;
    int llave, archivo;

    /* ------------------------- Memorias compartidas ------------------------- */

    /* Conexión al arreglo de asientos */
    asientos = (int *)shm(sizeof(int)*20, "shm_asientos", 'u');

    /* Conexión a la memoria compartida para la asignación de claves de comunicación con el hilo */
    claves_comunicacion_hilo = (int *)shm(sizeof(int)*2, "shm_parametros_cliente", 'v');

    /* ------------------------------ Semáforos ------------------------------- */

    /* Semáforo del servidor */
    sem_servidor = sem(0, "sem_servidor", 'w');

    /* Semáforo del cliente */
    sem_cliente = sem(0, "sem_cliente", 'x');

    /* Semáforos de los asientos */
    for (contador = 0; contador < 20; contador++) {
        /* Creación del nombre del archivo de la clave del asiento en el índice @contador */
        sprintf(numero_asiento, "%d", contador+1);
        strcpy(archivo_asiento, "sem_asiento_");
        strcat(archivo_asiento, numero_asiento);
        
        asientos[contador] = sem(1, archivo_asiento, contador+100);
    }

    /* --------------------------- Configuraciones ---------------------------- */


    /* ----------------- Obtención de claves de comunicación ------------------ */

    up(sem_servidor);
    printf("Obteniendo llaves...\n");
    down(sem_cliente);

    llave = claves_comunicacion_hilo[0];
    archivo = claves_comunicacion_hilo[1];

    up(sem_servidor);

    printf("Llaves obtenidas...\n");
    printf("shm: %c, sem: %d\n", llave, archivo);

    return 0;
}
