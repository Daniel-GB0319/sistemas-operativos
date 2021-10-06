#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>



int main(int argc, char *argv[]) {
	
	int x,totalAsientos,n;
	char asientos[10],aux[10];
	pid_t pidValor;
	int fd[2];
	
	pipe(fd);
	
	// Se inicializan todos los asientos a Libre
	for (x=0;x<n;x++){
				asientos[x]='L';
	}	
	 
	 // Se crea primer hijo
	pidValor = fork();
	 
	if (pidValor>=0){ // Es el Padre
		/*Se inicializan los valores del arreglo dinamico a Libre (L) por defecto*/
		printf("SOY EL PADRE = %d\n",getpid());

		for (x=0;x<n;x++){
				asientos[x]='L';
		}		
				write(fd[1],asientos, sizeof(asientos));// SE ESCRIBE EN TUBERIA
				asientos = read(fd[0],aux,sizeof(aux)); // SE LEE DE TUBERIA
	}else if(pidValor==0){ // Es el hijo
			printf("SOY EL HIJO = %d y MI PADRE ES = %d\n",getpid(),getppid());
			aux = read(fd[0],asientos,sizeof(asientos)); // SE LEE DE TUBERIA

			/*Se muestran los asientos del avion*/
			printf("********** Aerolinea ESCOM **********\n\n");
			printf("DISPONIBILIDAD DE ASIENTOS:\n");
			printf("L = Libre\t O = Ocupado\n");
			
			for (x=0;x<n;x++){
				printf("Asiento #%i\tESTADO: %c\n",x,asientos[x]);
			}
			
			// Se pregunta cuantos asientos desean reservar
			printf("Ingrese el NUMERO de Asientos que desea reservar: \n");
			scanf("%d",&totalAsientos);
			n=0;
			
			if (totalAsientos<=1 && totalAsientos<=10){ // El cliente ha ingresado un numero valido de asientos libres
					do{ // Se pediran los asientos que se reservaran
						printf("Ingrese el NUMERO que corresponde al Asiento que desea reservar: \n");
						scanf("%d",&x);
						
						if (asientos[x]=='O'){
							printf("!! El Asiento #%d ya esta Ocupado !!! \n",x);
						}else{
							asientos[x]='O';
							printf("!! El Asiento #%d ha sido seleccionado para reservar !!! \n",x);
						}	
						n++;
					}while(n==totalAsientos);
					
					// Se pide la confirmacion para la reserva
					printf(" Desea confirmar la reserva de los asientos seleccionados? (SI = 1 / NO = 0) (Ingrese el Numero Correspondiente a su respuesta) \n");
					scanf("%d",&x);
					
					if(x==1){ // Reserva Confirmada
						write(fd[1],asientos,sizeof(asientos)); // SE ESCRIBE EN TUBERIA
						printf("!!! Asientos Reservados con Exito !!! \n");
						printf("!!! Gracias por utilizar el Servicio de Reserva de Asientos !!!\n!!!Hasta la Proxima !!!\n");
						exit(0);
					}else{ // Reserva Anulada
						write(fd[1],aux,sizeof(aux)); // SE ESCRIBE EN TUBERIA
						printf("!!! Gracias por utilizar el Servicio de Reserva de Asientos !!!\n!!!Hasta la Proxima !!!\n");	
					}
					
			}else{
					if(totalAsientos==0){ // El cliente ha decidido no reservar asientos
						printf("!!! Gracias por utilizar el Servicio de Aparatado de Asientos !!!\n!!!Hasta la Proxima !!!\n");
						exit(0);	
					}else{ // El cliente ha ingresado un numero invalido de asientos
						
					}
				} 
		
		} // Termina Proceso Hijo
			
	return 0;
}
