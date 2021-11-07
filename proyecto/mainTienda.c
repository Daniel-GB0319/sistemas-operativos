#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "carrito.c"

// Prototipos
void menuCliente();
void menuAdmin();
void menuProveedor();

// Variables Globales
lista catalogo,cartClient;
carrito auxList;
int server,menu,submenu,pos,i,j,condicional,total;

int main(int argc, char *argv[]) {
	
	//Se crean las listas a utilizar
	crearlista(&catalogo); // Catalogo de Productos de la Tienda
	crearlista(&cartClient); // Carrito de Cliente 1
	
	//Se precargan productos en la lista "Catalogo de Productos"
	strcpy(auxList.name,"Cuaderno");
	auxList.cant=3;
	auxList.precio=10;
	add(0,auxList,catalogo); 

	strcpy(auxList.name,"Boligrafo");
	auxList.cant=3;
	auxList.precio=5;
	add(1,auxList,catalogo);
	
	strcpy(auxList.name,"Libro");
	auxList.cant=3;
	auxList.precio=20;
	add(2,auxList,catalogo);
	
	strcpy(auxList.name,"Mochila");
	auxList.cant=3;
	auxList.precio=10;
	add(3,auxList,catalogo);
	
	strcpy(auxList.name,"Uniforme");
	auxList.cant=3;
	auxList.precio=15;
	add(4,auxList,catalogo);

	do{
		printf("\t*************** Proyecto Tienda Online ***************\n\n");
		printf ("********************** MENU PRINCIPAL *************************\n\n"); 
		printf("!!! Bienvenido a Escommerce !!!\n\n");
		printf("1) Acceder como Admin \n2) Acceder como Proveedor \n3) Acceder como Cliente \n0) Salir de la Cuenta Cliente\n\n");	
		printf("Ingrese el NUMERO de la opcion deseada y presione ENTER ~& ");
		scanf("%i",&server);

		switch (server){
			case 1:
				/* AQUI VA FUNCION DE ADMIN QUE PUEDE AGREGAR Y BORRAR PRODUCTOS, ADEMAS DE CAMBIAR TODAS LAS CONTRASENAS */
			break;

			case 2:
				// AQUI VA FUNCION DE PROVEEDOR QUE PUEDE AGREGAR Y BORRAR SUS PRODUCTOS (FALTA REFERENCIAR EN STRUCT Y CODIGO GENERAL) 
				//ADEMAS DE CAMBIAR SU PROPIA CONTRASENAS
			break;

			case 3:
				menuCliente();
			break;
		
		default:
			break;
		}

	} while (server!=0);
	

	
	vaciarlista(cartClient);
	vaciarlista(catalogo);
	return 0;
}


// ### Funciones ###

// --- Clientes ---
void menuCliente(){
	do{// Comienza Menu Principal del Programa
		printf("\t*************** Proyecto Tienda Online - Cliente ***************\n\n");
		printf ("**********************MENU PRINCIPAL*************************\n\n"); 
		printf("!!! Bienvenido a Escommerce !!!\n\n");
		printf("1) Mostrar Catalogo de Tienda\n2) Gestionar Carrito\n0) Salir de la Cuenta Cliente\n\n");	
		printf("Ingrese el NUMERO de la opcion deseada y presione ENTER ~& ");
		scanf("%i",&menu);
	
		/* PROBABLEMENTE SE CONFIRME COMPRA AL DAR OPCION SALIR DE LA CUENTA CLIENTE, SE MOSTRARA EL CARRITO Y CALCULARA EL TOTAL
		SI CLIENTE DECIDE NO COMPRAR, SE DARA ADVERTENCIA DE QUE SE BORRARA SU CARRITO
		SI NO COMPRA, TODOS LOS PRODUCTOS DE CARRITO VOLVERAN A CATALOGO (PROCESO SIMILAR A BUSCAR SI HAY PRODUCTOS EN CARRITO EN OPCION AGREGAR PRODUCTO)
		*/
		switch (menu){ // Operaciones para el MENU PRINCIPAL
			case 1: // 1) Mostrar Catalogo
				printf ("\n***Catalogo de Productos Disponibles***\n");    
				for(i=0;i<catalogo->NE;i++){ // Se imprimen los elementos de la lista "Catalogo de Productos Disponibles"
	  				printf("Producto #%d: %s\n",i,get(i,catalogo).name);
					printf("Cantidad: %d Unidades.\n",i,get(i,catalogo).cant);
					printf("Precio: $%d Pesos.\n\n",i,get(i,catalogo).precio);   
				}
				printf("!!! Fin del Catalogo !!! Presione Enter para Continuar...\n");	  
				getchar(); getchar(); system("clear");
			break;
			case 2: // 2) Ver Carrito
				system("clear");
				do{ // Inicia Submenu Gestionar Carrito
					printf("\t*************** Proyecto Tienda Online - Cliente ***************\n\n");
					printf ("********************** SUBMENU GESTIONAR CARRITO *************************\n\n"); 
					printf("1)Ver Carrito \n2) Agregar un Producto al Carrito \n3) Borrar un Producto del Carrito \n0) Regresar al Menu Principal\n\n"); // Opciones 	
					printf("Ingrese el NUMERO de la operacion deseada y presione ENTER ~& ");
					scanf("%i",&submenu);
									
					switch(submenu){
						case 1: // Opcion Ver Carrito
							if (!empty(cartClient)){ // Carrito Tiene Productos
								for(i=0;i<catalogo->NE;i++){ // Se imprimen los elementos de la lista "Catalogo de Productos Disponibles"
									printf("Producto #%d: %s\n",i,get(i,catalogo).name);
									printf("Cantidad: %d Unidades.\n",i,get(i,catalogo).cant);
									printf("Precio: $%d Pesos.\n\n",i,get(i,catalogo).precio);   
								}
								printf("!!! Fin del Carrito !!! Presione Enter para Continuar...\n");	  
								getchar(); getchar(); system("clear");
							}else{ // Carrito esta Vacio
								printf("\n!!! El Carrito esta vacio !!! Presione Enter para continuar...\n"); getchar(); getchar(); system("clear");	
							}	
						break;
											
						case 2: // Opcion Agregar un Producto al Carrito 
							printf ("\n***Catalogo de Productos Disponibles***\n");    
							for(i=0;i<catalogo->NE;i++){ // Se imprimen los elementos de la lista "Catalogo de Productos Disponibles"
								printf("Producto #%d: %s\n",i,get(i,catalogo).name);
								printf("Cantidad: %d Unidades.\n",i,get(i,catalogo).cant);
								printf("Precio: $%d Pesos.\n\n",i,get(i,catalogo).precio);   
							}
							printf("Ingrese el NUMERO del producto que desea agregar al carrito y presione ENTER ~& ");
							scanf("%i",&i);
							if (i>=0 && i<=catalogo->NE){
								// Se verifica si carrito esta vacio
								if(!empty(cartClient)){ // Carrito no Vacio
									// Se verifica si existe el mismo producto elegido del catalogo en el carrito
									for(i=0;j<catalogo->NE;j++){ // Recorre cada producto del catalogo
										for(i=0;i<cartClient->NE;i++){
											if(get(j,catalogo).name==get(i,cartClient).name && get(j,catalogo).cant==get(i,cartClient).cant && get(j,catalogo).precio==get(i,cartClient).precio){ 
											// Se encontro el mismo producto en carrito, se aumentara la cantidad en 1
												// Se actualiza cantidad de producto en carrito +1
												strcpy(auxList.name,get(i,cartClient).name);
												auxList.cant=get(i,cartClient).cant +1;
												auxList.precio=get(i,cartClient).precio;
												
												borrar (i,cartClient);
												add (i,auxList,cartClient);
												
												// Se actualiza cantidad de producto en Catalogo -1
												strcpy(auxList.name,get(j,catalogo).name);
												auxList.cant=get(j,catalogo).cant +1;
												auxList.precio=get(j,catalogo).precio;
												
												borrar (j,catalogo);
												if(auxList.cant>=1) add (j,auxList,catalogo);

												condicional=1;
											}
										}
									}
								}
								if(empty(cartClient) || condicional==1){ // Producto nuevo en carrito
									strcpy(auxList.name,get(i,catalogo).name);
									auxList.cant=1;
									auxList.precio=get(i,catalogo).precio;
						
									printf("\nIngrese el NUMERO de la POSICION donde se desea agregar el producto a su carrito y presione ENTER: ");
									scanf("%i",&pos); //Se ingresa la posicion
						
									if (pos>=0){
										add (pos,auxList,cartClient); // Se agrega producto a carrito
										getchar(); getchar(); system("clear");
									}else{
										printf("\n!!!Ha insertado una rango de Posicion invalida!!! Intente de nuevo\n"); getchar(); getchar(); system("clear");  
									}
								}
								

							} else{ // Opcion invalida en catalogo
								printf("\n!!!Ha insertado una opcion invalida!!! Intente de nuevo\n"); getchar(); getchar(); system("clear");
							}
						break;

						case 3: // Opcion Borrar un Producto del carrito
							if (!empty(cartClient)){ // Se comprueba si no esta vacio el carrito
							// Existen productos en carrito
								printf ("\n*** Catalogo de Productos Disponibles en Carrito ***\n");    
								for(i=0;i<catalogo->NE;i++){ // Se imprimen los elementos de la lista "Catalogo de Productos Disponibles"
									printf("Producto #%d: %s\n",i,get(i,catalogo).name);
									printf("Cantidad: %d Unidades.\n",i,get(i,catalogo).cant);
									printf("Precio: $%d Pesos.\n\n",i,get(i,catalogo).precio);   
								}

								printf("\nSeleccione la POSICION del producto que desea Borrar del Carrito\n");
								scanf("%i",&pos);//Se ingresa la posicion
								if (pos>=0 && pos<cartClient->NE){
									borrar (pos,cartClient); // Se borra producto de carrito
									printf("\n!!! Producto borrado del carrito con Exito !!! Presione Enter para Continuar...\n"); getchar(); getchar(); system("clear");
								} else {
									printf("\nPOSICION fuera de Rango\n");
								}												
								getchar(); getchar(); system("clear");
							}else{
								printf("\n!!! El Carrito esta vacio !!! Presione Enter para Continuar...\n"); getchar(); getchar(); system("clear");
							}
						break;

						case 0: system("clear"); break; // Regresar al Menu Principal
											
						default: printf("\n!!!Ha insertado una opcion invalida!!! Intente de nuevo\n"); getchar(); getchar(); system("clear");
					} //Termina switch submenu
				
				}while(submenu!=0); system("clear");
			break;
			case 0: // Mensaje al salir del programa
				printf("\n!!! Hasta la proxima !!!");
			break;
			default: printf("\n!!!Ha insertado una opcion invalida!!! Intente de nuevo\n"); getchar(); getchar(); system("clear");	
		}
	}while(menu!=0); 
}