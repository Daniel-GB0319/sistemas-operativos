#include <stdio.h>
#include "../include/systemv.h"

int main(void)
{
    int sem_servidor;
    int sem_cliente;
    int *claves_comunicacion_hilo;
    int llave, archivo;

    /* ------------------------- Memorias compartidas ------------------------- */

    /* Conexión a la memoria compartida para la asignación de claves de comunicación con el hilo */
    claves_comunicacion_hilo = (int *)shm(sizeof(int)*2, "shm_parametros_cliente", 'v');

    /* ------------------------------ Semáforos ------------------------------- */

    /* Semáforo del servidor */
    sem_servidor = sem(0, "sem_servidor", 'w');

    /* Semáforo del cliente */
    sem_cliente = sem(0, "sem_cliente", 'x');

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
