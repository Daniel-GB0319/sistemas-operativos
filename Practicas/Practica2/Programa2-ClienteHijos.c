#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

int main(){
    int fd, n, lugar, i, boletos, respCliente, buf[10];

    printf("\n***** Practica2 -- Procesos *****\n");
    printf("\n-- Sistema de Compra de Boletos --\n");
    
    // Cliente avisa a Servidor que se ha conectado
    mkfifo("/tmp/pract2_hijos", 0666);
    fd = open("/tmp/pract2_hijos", O_WRONLY);
    write(fd, &buf[0], 1);
    close(fd);

    // Se imprimen los lugares
    printf("\nLos lugares para el vuelo son:\n");
    printf("1 = Disponible para Compra / 0 = Ocupado\n\n");
    
    fd = open("/tmp/pract2_hijos", O_RDONLY);
    for (int i = 0; i < 5; i++){
        n = read(fd, &buf[i], sizeof(int));
        printf("Lugar #%d: %d\n",i, buf[i]);
    };
    close(fd);

    //Confirmación de compra de boletos
    printf("\n\nDesea comprar boletos para el vuelo? (Si=1 / No=0)\n");
    printf("Ingrese el numero correspondiente a la opcion deseada: ");
    scanf("%d", &respCliente);

    // Se informa al servidor sobre la confirmación del cliente
    mkfifo("/tmp/pract2_hijos", 0666);
    fd = open("/tmp/pract2_hijos", O_WRONLY);
    write(fd, &respCliente, 1);
    close(fd);

    if (respCliente == 1){// Cliente quiere comprar boletos
        printf("\n\nCuantos boletos desea comprar?: ");
        scanf("%d", &boletos);
        
        for (i = 0; i < boletos; i++){
            printf("\n# del Lugar que desea Comprar: \n");
            scanf("%d", &lugar);
        //se verifica si lugar esta ocupado o no
            if(buf[lugar]==1){
                buf[lugar]=0;
            }else{
                printf("No se puede comprar ese lugar. Ya esta Ocupado\n");
            }
        }
        // Se envian los boletos comprados a Servidor
        mkfifo("/tmp/pract2_hijos", 0666);
        fd = open("/tmp/pract2_hijos", O_WRONLY);
        for (i = 0; i < 5; i++){
            write(fd, &buf[i], sizeof(int));
        }
        close(fd);

        printf("Gracias por comprar boletos. Vuelva Pronto\n\n");
    }else{ // No se desea comprar boletos
        printf("Gracias por revisar el catalogo de boletos. Vuelva Pronto\n\n");
    }
    return 0;
}