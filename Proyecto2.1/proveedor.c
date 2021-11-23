#include <stdio.h>
#include "./include/systemv.h"

int main(void)
{
    int contador;
    int sem_servidor;
    int sem_proveedor;
    char numero_acceso[3]; // Para convertir el número de acceso a cadena
    char archivo_acceso[15]; // Para almacenar el nombre del archivo para la llave de cada asiento
    int *acceso = NULL;
    int *claves_comunicacion_hilo;
    int llave, archivo;

    /* ------------------------- Memorias compartidas ------------------------- */

    /* Conexión al arreglo de acceso */
    acceso = (int *)shm(sizeof(int)*20, "shm_acceso", 'u');
    
    /* Conexión a la memoria compartida para la asignación de claves de comunicación con el hilo */
    claves_comunicacion_hilo = (int *)shm(sizeof(int)*2, "shm_parametros_proveedor", 'v');
    /* ------------------------------ Semáforos ------------------------------- */

    /* Semáforo del servidor */
    sem_servidor = sem(0, "sem_servidor", 'w');

    /* Semáforo del cliente */
    sem_proveedor = sem(0, "sem_proveedor", 'x');

    /* Semáforos de los acceso */
    for (contador = 0; contador < 20; contador++) {
        /* Creación del nombre del archivo de la clave del asiento en el índice @contador */
        sprintf(numero_acceso, "%d", contador+1);
        strcpy(archivo_acceso, "sem_acceso_");
        strcat(archivo_acceso, numero_acceso);
        
        acceso[contador] = sem(1, archivo_acceso, contador+100);
    }

    /* --------------------------- Configuraciones ---------------------------- */


    /* ----------------- Obtención de claves de comunicación ------------------ */

    up(sem_servidor);
    printf("Obteniendo llaves...\n");
    down(sem_proveedor);

    llave = claves_comunicacion_hilo[0];
    archivo = claves_comunicacion_hilo[1];

    up(sem_servidor);

    printf("Llaves obtenidas...\n");
    printf("shm: %c, sem: %d\n", llave, archivo);

    return 0;
}
