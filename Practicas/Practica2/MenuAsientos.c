#include <stdio.h>
#include <stdlib.h>

/* run this program using the console pauser or add your own getch, system("pause") or input loop */

int main(int argc, char *argv[]) {
	
	int n=10,x,opcion;
	char *asientos;
	
	printf("\t***Gonzalez Barrientos Geovanni Daniel \t1CV3 \tEstructuras de Datos***\n\n");
	 /* Se crea el arreglo dinamico*/
	asientos= (char*) malloc(n*sizeof(char));
	
	/*Se verifica que exista espacio disponible en memoria para el arreglo*/
	if (asientos==NULL){
		printf("***No se ha podido reservar suficiente memoria***");
		exit(0);
	}
	else{
		/*Se inicializan los valores del arreglo dinamico a Libre (L) por defecto*/
		for (x=0;x<n;x++){
				*(asientos + x)='L';
			}
		/*El programa se ejecutara continuamente hasta que el usuario desee salir*/ 
		do{
			/*Se muestra el menu principal*/
			printf("**********MENU PRINCIPAL**********\n\n");
			printf("DISPONIBILIDAD DE ASIENTOS:\n");
			
			for (x=0;x<n;x++){	/* Se muestra la disponibilidad de los asientos*/
				printf("Asiento #%i\tESTADO: %c\n",x+1,*(asientos + x));
			}
			printf("\nOPCIONES DEL MENU:\n");
			printf("1) Reservar un lugar/asiento\n");
			printf("2) Cancelar la reservacion de un lugar/asiento\n");
			printf("0) Salir del Programa\n\n");
			printf("Ingrese el NUMERO correspondiente a la opcion que desea realizar: ");
			scanf("%i",&opcion);
			
			switch(opcion) {
				case 1:
					printf("Ingrese el numero del asiento que desea reservar: ");
					scanf("%i",&x);
					x--;
					if(*(asientos + x)=='L'){
						*(asientos + x)='O';
						printf("El asiento ha sido reservado con Exito\n");
						system("pause");
					}
					else{
						printf("El asiento que ha seleccionado ya esta Ocupado\n");
						system("pause");
					}
				break;
				
				case 2:
					printf("Ingrese el numero del asiento que desea cancelar su reservacion: ");
					scanf("%i",&x);
					x--;
					if(*(asientos + x)=='O'){
						*(asientos + x)='L';
						printf("La reservacion ha sido cancelado con Exito\n");
						system("pause");
					}
					else{
						printf("El asiento que ha seleccionado no tiene ninguna reservacion\n");
						system("pause");
					}
				break;
				
				case 0:
					printf("****HASTA LA PROXIMA*****");
					free(asientos);
					exit(0);
				default: 
					printf("Ha ingresado una opcion invalida. Intente de nuevo");
					system("pause");
			}
		}while(opcion!=0);
	}

	free(asientos);
	return 0;
}
