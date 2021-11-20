#ifndef _SYSTEMV_H_
#define _SYSTEMV_H_

void touch(char archivo[]);
void *shm(int tamano, char archivo[], char clave);
void shmfree(void *shm, char archivo[], char clave);
int sem(int valor_inicial, char archivo[], char clave);
int semval(int semid);
void semfree(int semid, char archivo[], char clave);
void down(int semid);
void up(int semid);

#include "../src/systemv.c"

#endif /* _UNIDADV_H_ */
