#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

//Variables globales
int fd;
int buf[10000];
char bufc[10000];
int personas, Confirmacion;

void piper(int fd) //Recepción de asientos
{
    int n;
    fd = open("/tmp/mi_fifo", O_RDONLY);
    for (int i = 0; i < 5; i++){
        n = read(fd, &buf[i], sizeof(int));
        printf("Lugar #%d: %d\n",i, buf[i]);
    };
    close(fd);
}

void pipew(int fd, int buf[]) //Tubería de asignación de asientos
{
    mkfifo("/tmp/mi_fifo", 0666);
    fd = open("/tmp/mi_fifo", O_WRONLY);
    for (int i = 0; i < personas; i++){
        write(fd, &buf[i], sizeof(int));
    }
    close(fd);
}

void piperc(int fd){ //Pipe de Confirmacion
    fd = open("/tmp/mi_fifo", O_RDONLY);
    read(fd, bufc, sizeof(bufc));
    printf("\nEstatus: %s\n", bufc);
    close(fd);
}

void pipemenu(int Confirmacion){
    mkfifo("/tmp/mi_fifo", 0666);
    fd = open("/tmp/mi_fifo", O_WRONLY);
    write(fd, &Confirmacion, sizeof(int));
    close(fd);
}

void menu(){
    printf("\n\t*** Practica2 -- Procesos ***\n");
    printf("\n\t*** Compra de Boletos ***\n");
    // Cliente avisa a Servidor que se ha conectado
    mkfifo("/tmp/mi_fifo", 0666);
    fd = open("/tmp/mi_fifo", O_WRONLY);
    write(fd, &buf[0], 1);
    close(fd);

    //Tubería de recepción de asientos
    printf("\nLos lugares para el vuelo son:\n");
    printf("1 = Disponible para Compra / 0 = Ocupado\n\n");
    piper(fd);
    printf("\n");

    //Confirmación de vuelos
    printf("\nPor favor confirme si desea adquirir los boletos (1) para si (2) para no\n");
    scanf("%d", &Confirmacion);

    //pipe de menu de confirmación
    pipemenu(Confirmacion);

    if (Confirmacion == 1){
        //Tubería de reserva de asientos
        printf("Indique el numero de personas para el vuelo:\n");
        scanf("%d", &personas);
        for (int i = 0; i < personas; i++){
            printf("Asiento del Pasajero %d:\n", i);
            scanf("%d", &buf[i]);
        }
        pipew(fd, buf);

        //Tubería de confirmación de estatus de la reserva
        piperc(fd);
    }else{
        menu();
    }
}

int main(void)
{
    menu();
    return 0;
}