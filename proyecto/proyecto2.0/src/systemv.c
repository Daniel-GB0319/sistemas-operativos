#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#include "../include/systemv.h"

/**
 * touch - crear un archivo en el directorio actual del programa
 * 
 * @archivo: el nombre del archivo
 * 
 * Se genera y ejecuta el comando touch para crear un archivo
 * nombrado @archivo en el directorio donde se ejecuta el programa.
 */
void touch(char archivo[])
{
	char comando[strlen(archivo)+7];

	strcpy(comando, "touch ");
	strcat(comando, archivo);
	system(comando);
}

/**
 * shm - obtener un espacio de memoria compartida
 * 
 * @tamano: tamaño de la memoria compartida
 * @archivo: nombre del archivo para generar la llave
 * @clave: clave para generar la llave
 */
void *shm(int tamano, char archivo[], char clave)
{
	int shmid;
	key_t llave;

	touch(archivo); // El archivo de la llave es creado
	llave = ftok(archivo, clave); // Se obtiene la llave
	shmid = shmget(llave, tamano, IPC_CREAT|0600); // Se obtiene la memoria compartida

	return shmat(shmid, 0, 0);
}

/**
 * shmfree - liberar memoria compartida
 * 
 * @shm: el apuntador a la memoria compartida
 * @archivo: nombre del archivo de la llave de la memoria compartida
 * @clave: la clave de la llave de la memoria compartida
 */
void shmfree(void *shm, char archivo[], char clave)
{
	int shmid;
	key_t llave;

	llave = ftok(archivo, clave); // Se obtiene la llave de la memoria compartida
	shmid = shmget(llave, (size_t)NULL, IPC_CREAT|0600); // Se obtiene el id de la memoria compartida

	/* Se libera la memoria compartida */
	shmdt(shm);
	shmctl(shmid, IPC_RMID, NULL);
	unlink(archivo); // El archivo de la llave es eliminado
}

/**
 * sem - obtener un semáforo
 * 
 * @valor_inicial: el valor inicial del semáforo
 * @archivo: nombre del archivo para generar la llave
 * @clave: clave de la llave
 */
int sem(int valor_inicial, char archivo[], char clave)
{
	int semid;
	key_t llave;

	touch(archivo); // Se crea el archivo de la llave
	llave = ftok(archivo, clave); // Se obtiene la llave
	semid = semget(llave, 1, IPC_CREAT|0644); // Se obtiene el semáforo

	if (semid == -1) // ¿Fue posible obtener el semáforo?
		return -1;

	semctl(semid, 0, SETVAL, valor_inicial); // El valor inicial sel semáforo es configurado

	return semid;
}

/**
 * semval - obtener el valor actual de un determinado semáforo
 * 
 * @semid: el id del semáforo
 */
int semval(int semid)
{
	return semctl(semid, 0, GETVAL);
}

/**
 * semfree - liberar un semáforo
 * 
 * @semid: el id del semáforo
 * @archivo: el nombre del archivo de la llave
 * @clave: la clave de la llave
 */
void semfree(int semid, char archivo[], char clave)
{
	semctl(semid, 0, IPC_RMID); // El semáforo es liberado
	unlink(archivo); // Se elimina el archivo de la llave
}

void down(int semid)
{
	struct sembuf op_p[] = {0, -1, 0};
	semop(semid, op_p, 1);
}

void up(int semid)
{
	struct sembuf op_v[] = {0, +1, 0};
	semop(semid, op_v, 1);
}
