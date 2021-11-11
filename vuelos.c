#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int matriz_reservados[6][6];
char letras[6] = {'A', 'B', 'C', 'D', 'E'};
char asiento[2] = "";
int fila, columna;
int b;
int contador=0;
void asientos_disponibles(int matriz_asientos[6][6]);
void reservar_asiento(int matriz_asientos[6][6], char *letras[6]);
void clsc(void);

int main(void){
    int i,j;

    for(i=0; i<6; i++){
        for(j=0; j<6; j++){
            matriz_reservados[i][j] = 0;
        }
    }

    char n_vuelo[5];
    int opcion;

    start:
        printf("\nBienvenido a Mexicana\n");
        printf("\n\n");
        printf("Ingrese el numero de vuelo : (5 digitos)  \n");
        scanf("%s", &n_vuelo);


        if(strlen(n_vuelo) == 5){
            menu: 
                clsc();
                printf("Bienvenido al menu de reservacion del vuelo %s\n", n_vuelo);
                printf("\nSeleccione una opcion: \n" );
                printf("1. Reservar asiento\n");
				printf("2. Ver asientos disponibles\n");
				printf("3. Ver resumen\n");
				printf("4. Salir\n ");

				printf("\n\n\n");
				printf("Opcion elegida:\n");
                scanf("%d", &opcion);
                printf("\n");

                if(opcion == 1){
                    scanf("%c", &b);
                    clsc();

                    reserva: 
                        printf("Reserva de asientos \n\n\n");
                        printf("Eliga una opcion entre A-F, 1-6\n");
                        printf("Ingrese el asiento que desea reservar ( (menu) para regresar):");
                        scanf("%s", &asiento);
                        scanf("%c", &b);

                        if(strlen(asiento) == 2){
                            columna = asiento[0] - 65;
                            fila = asiento[1] - 49;

                            if(fila > 5 || columna > 5){
                                printf("Asiento invalido\n");
                                goto reserva;
                            } else if(matriz_reservados[columna][fila] == 0){
                                matriz_reservados[columna][fila] = 1;
                                contador++;
                                printf("Asiento %s reservado exitosamente!\n\n", asiento);
                                printf("\n\n");
                                printf("Presione enter para regresar");
                                scanf("%c", &b);
                                goto menu;
                            } else {
                                clsc();
                                printf("El asiento solicitado ya esta reservado. \n");
                                printf("Seleccione otro asiento\n");
                                printf("\n\n");
                                goto reserva;
                            }
                        } 
					    else if(strcmp(asiento, "menu") == 0){
                            goto menu;
					    }
						else {
                            goto reserva;
                        }

                } else if(opcion == 2){
                    scanf("%c", &b);
                    disponibilidad:
                        printf("Asientos disponibles = o\n");
                        printf("Asientos ocupados = x\n");
                        printf("Listado de Asientos\n");

                        for(i = 1; i<7; i ++){
                            printf("  %d ", i);
                        }
                        printf("\n");

                        for(i = 0; i<6; i++){
                            printf("%c", i+65);
                            for(j=0; j<6; j++){
                                if(matriz_reservados[i][j] == 1){
                                    printf(" [x]");
                                } else {
                                    printf(" [o]");
                                }
                            }
                            printf("\n");
                        }
                        printf("Presione enter para regresar");
                        scanf("%c", &b);
                        goto menu;

                } else if(opcion == 3){
                    scanf("%c", &b);

                    Resumen:
                        printf("Estado del Vuelo: \n");
                        printf("Asientos disponibles: %d (%d%c)\n", 36-contador, 100 - ((contador * 100)/36), 37);
                        printf("Asientos reservados: %d (%d%c)\n", contador, (contador * 100)/36, 37);
                        printf("Presione enter para regresar");
                        scanf("%c", &b);
                        goto menu;

                } else if(opcion == 4){
                    return 0;

                } else {
                    goto menu;
                }

        } else {
            goto start;
        }
}

void clsc(void){
    system("@cls||clear");
}
