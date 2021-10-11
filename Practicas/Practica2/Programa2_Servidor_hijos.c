//Generales y fifo
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <wait.h>
#include <sys/types.h>

//Variables globales
int fd = -1;
pid_t pid;
int asientos[5];
char confirm[] = "Vuelo y asientos reservados correctamente";
char asientooc[] = "El asiento esta ocupado";
int respClient;

int binarySearch(int arr[], int l, int r, int x){
    while (l <= r){
        int m = l + (r - l) / 2;
        int dato = 0;
        // Check if x is present at mid
        if (arr[m] == x){ //Se verifica si el valor buscado esta en la m-esima posicion y regresa el dato
            dato = x;
            arr[m] = 0;
            return dato;
        }
        // If x greater, ignore left half

        if (arr[m] < x) //Se verifica si el valor buscado es mayor al valor en la m-esima posicion
            l = m + 1;  //Se delimita la parte izquierda del nuevo arreglo a analizar, para descartar los valores en donde no se encuentra el valor buscado
        // If x is smaller, ignore right half
        else           //Se verifica si el valor buscado es menor al valor en la m-esima posicion
            r = m - 1; //Se delimita la parte derecha del nuevo arreglo a analizar, para descartar los valores en donde no se encuentra el valor buscado
    }
    // if we reach here, then element was
    // not present
    return -1;
}

void pipera(int fd, int asientos[]) //Lectura de la reserva de asientos el 5 es el tamaño de los asientos
{
    int buf[10000];
    int reserva[10000];
    int n;
    fd = open("/tmp/mi_fifo", O_RDONLY);
    for (int i = 0; i < 5; i++)
    {
        n = read(fd, &buf[i], sizeof(int));
        reserva[i] = binarySearch(asientos, 0, 5, buf[i]);
        if (reserva[i] != -1){
            printf("Asiento del Cliente: %d\n", reserva[i]);
        }
    }
    for (int i = 0; i < 5; i++){
        printf("\nAsiento disponible %d\n", asientos[i]);
    }
    close(fd);
}

int main(void){
    int buf[10000];
    int n, x = 0 ;

    // se inicializan lugares a disponibles
    for(n=0;n<5;n++){
        asientos[n]=1;
    }
    n=0;
    //Bucle de servidor Infinito 
    while (x != 1){
        printf("SERVIDOR ACTIVO...");
        fd = open("/tmp/mi_fifo", O_RDONLY);
        n = read(fd, &buf[0], sizeof(int));
        
        if(n==1){ // Cliente se ha conectado
            pid = fork();
            if (pid == -1){
                perror("\nError al crear el proceso\n");
                exit(-1);
            }
        if (pid == 0){
            printf("\nSoy el Hijo = %d y mi Padre es = %d\n",getpid(),getppid());      
            printf("!!! Cliente Conectado !!!");
            
            //Se envian los boletos disponibles
            mkfifo("/tmp/mi_fifo", 0666);
            fd = open("/tmp/mi_fifo", O_WRONLY);
            for (int i = 0; i < 5; i++){
                write(fd, &asientos[i], sizeof(int));
            }
            close(fd);

            printf("\nLista de Boletos Disponibles Enviados\n");
            
            // Se comprueba respuesta de cliente sobre comprar o no boletos
            fd = open("/tmp/mi_fifo", O_RDONLY);
            n = read(fd, &respClient, sizeof(int));
            if (respClient == 1){ // Cliente compra
                printf("/nCliente ha decidido comprar Boletos/n/n");
                pipera(fd, asientos); //tuberia de envio de asientos disponibles
            }else{
                printf("/nCliente no quiere comprar Boletos/n/n");
                wait(NULL);
                printf("Soy el Padre = %d\n",getpid());
            }
        }else { // Es el padre   
        printf("\nSoy el Padre = %d\n",getpid() );
    
        } // Termina mensajes Padre
        } // Termina proceso para el cliente
        sleep(3000);
        system("clear");
    } // Termina Bucle While
    return 0;
}