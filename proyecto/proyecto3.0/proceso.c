#include <stdio.h>
#include "include/systemv.h"

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

    comunicacion[0] = 0; // Pasamos el tipo de proceso al servidor

    up(sem_servidor);
    printf("Obteniendo llaves...\n");
    down(sem_proceso);

    clave = comunicacion[1];
    archivo = comunicacion[2];

    up(sem_servidor);

    printf("Llaves obtenidas...\n");
    printf("Clave: %c, Archivo: %d\n", clave, archivo);

    return 0;
}
