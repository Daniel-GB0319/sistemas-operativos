#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <wait.h>
#include <sys/types.h>


int main(void){
    int buf[10], respClient, n, x = 0, fd = -1, status, asientos[5];
    pid_t pid;

    // se inicializan lugares a disponibles
    for(n=0;n<5;n++){
        asientos[n]=1;
    }
    n=0;

    //Bucle de servidor Infinito 
    while (x != 1){
        printf("SERVIDOR ACTIVO...");
        fd = open("/tmp/pract2_hijos", O_RDONLY);
        n = read(fd, &buf[0], sizeof(int));
        
        if(n==1){ // Cliente se ha conectado
            pid = fork();
            if (pid == -1){
                perror("\nError al crear el proceso\n");
                exit(-1);
            }
        if (pid == 0){ // Es el Proceso Hijo
            printf("\nSoy el Hijo = %d y mi Padre es = %d\n",getpid(),getppid());      
            printf("!!! Cliente Conectado !!!");
            
            //Se envian los boletos disponibles
            mkfifo("/tmp/pract2_hijos", 0666);
            fd = open("/tmp/pract2_hijos", O_WRONLY);
            for (int i = 0; i < 5; i++){
                write(fd, &asientos[i], sizeof(int));
            }
            close(fd);

            printf("\nLista de Boletos Disponibles Enviados\n");
            
            // Se comprueba respuesta de cliente sobre comprar o no boletos
            fd = open("/tmp/pract2_hijos", O_RDONLY);
            n = read(fd, &respClient, sizeof(int));
            if (respClient == 1){ // Cliente compra
                printf("\nCliente ha decidido comprar Boletos\n\n");
                // Se actualizan la disponibilidad de lugares
                fd = open("/tmp/pract2_hijos", O_RDONLY);
                for (int i = 0; i < 5; i++){
                    n = read(fd, &asientos[i], sizeof(int));
                    printf("\nLugar #%d : %d\n",i, asientos[i]);
                }
                close(fd);
                exit(0);
            }else{ // Cliente no compra
                printf("\nCliente no quiere comprar Boletos\n\n");
                exit(0);
            }
        }else { // Es el padre   
        printf("\nSoy el Padre = %d\n",getpid() );
        wait(&status);
        printf("\nProceso hijo finalizado");
        } // Termina mensajes Padre
        } // Termina proceso para el cliente
        sleep(3000);
        system("clear");
    } // Termina Bucle While
    return 0;
}