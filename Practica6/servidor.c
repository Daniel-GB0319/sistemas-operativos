#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/types.h>					
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wait.h>
#include "../include/systemv.h"

#define T 30


typedef struct //Estructura para almacenar la fecha.
{
	short int dia;
	short int mes;
	short int anio;
	
}fechaT;

typedef struct //Estructura para almacenar los datos del cliente.
{
	unsigned int cuenta;	 //4 bytes
	char nombre[T];			//30 bytes
	char apellido[T]; 		//30 bytes
	char correo[T+10];		//40 bytes
	char telefono[T-10];	//20 bytes
	double fondos;					//8 bytes
	short int nip;					//2 bytes
	fechaT fRe; //Fecha de registro //6 bytes
	fechaT fMo; //Fecha movimiento. //6 bytes
	
}clienteT;

void menu(void);

void administrador(void);
void agregarCliente(void);
void mostrarClientes(void);
void buscarCliente(void);
void ordenarClientesName(void);
void editarDatos(void);
void eliminarDatos(void);

unsigned int iniciarSesion(void);
void cliente(unsigned int);
void mostrarInformacion(unsigned int);
void realizarDeposito(unsigned int);
void realizarRetiro(unsigned int);
void transferir(unsigned int);

int main(void)
{	
	srand(time(NULL));	
	int contador;
    int sem_servidor;
    int sem_cliente;
	int sem_administrador;
    int llaves_conexion[20][2];
    char numero_acceso[3]; // Para convertir el numero de acceso a cadena
    char archivo_acceso[15]; // Para alamcenar el nombre del archivo para la llave de cada acceso
    int *acceso = NULL;
    int *parametros_cliente = NULL;
	int *parametros_administrador = NULL;

    /* ------------------------- Memorias compartidas ------------------------- */

    /* Arreglo de acceso */
    acceso = (int *)shm(sizeof(int)*20, "shm_acceso", 'u');

    /** 
     * Memoria compartida para asignar llaves de comunicacion al cliente
     * nuevo para que pueda comunicarse con el hilo que le es asignado
     */
    parametros_cliente = (int *)shm(sizeof(int)*2, "shm_parametros_cliente", 'v');
	/** 
     * Memoria compartida para asignar llaves de comunicaciÃ³n al administrador
     * nuevo para que pueda comunicarse con el hilo que le es asignado
     */
    parametros_administrador = (int *)shm(sizeof(int)*2, "shm_parametros_administrador", 'v');

    /* ------------------------------ SemÃ¡foros ------------------------------- */

    /* Semaforo del servidor */
    sem_servidor = sem(0, "sem_servidor", 'w');

    /* Semaforo del cliente */
    sem_cliente = sem(0, "sem_cliente", 'x');

	/* Semaforo del administrador */
    sem_administrador = sem(0, "sem_administrador", 'x');

    /* Llenado del arreglo de acceso */
    for (contador = 0; contador < 20; contador++) {
        /* Creacion del nombre del archivo de la clave del acceso en el indice contador */
        sprintf(numero_acceso, "%d", contador+1);
        strcpy(archivo_acceso, "sem_acceso_");
        strcat(archivo_acceso, numero_acceso);

        /**
         * Creacion del semaforo para el acceso en el idice contador.
         * 
         * Ya que los acceso son administrados por medio de un arreglo de enteros,
         * podemos utilizar cada elemento del arreglo como semaforo del acceso
         * correspondiente en el indice contador.
         */
        acceso[contador] = sem(1, archivo_acceso, contador+100);
    }

    /* --------------------------- Configuraciones ---------------------------- */

    /* Llenado del arreglo con las llaves de conexion para la memoria compartida con hilos */
    for (contador = 0; contador < 20; contador++) {
        llaves_conexion[contador][0] = contador + 97;
        llaves_conexion[contador][1] = true;
    }

    /* Hilos para atender */
    pthread_attr_t atributos;
	pthread_t hilos[20];
	pthread_attr_init (&atributos);
	pthread_attr_setdetachstate (&atributos, PTHREAD_CREATE_DETACHED);

	short int i, option, modo = 1; //Se inicializa en 1 para que entre en el while una vez
	unsigned int sesion;
	clienteT usuario;

	do{ // Inicia SERVIDOR
		system("clear"); //Limpiar la pantalla
		setbuf(stdin,NULL); //Limpiar el buffer
		
		printf("\n\t\t\t\t\t\t   M E N U ");
		printf("\n\t\t"); //Salto de linea y tabulador para mostrar la linea mas centrada.
		
		for(i=0; i<80; i++) //Ciclo para ir mostrando una linea de separacion.
			printf("*");
			
		printf("\n\n\t\t\t[1] Administrador\t\t\t\t[2] Cliente");
		printf("\n\n\n\n\t\t\t\t\t\t   [0] Salir");
		
		printf("\n\n\n\n\t\t\t\t\t\tModo de acceso: ");
		scanf("%hd",&modo);

		switch(modo)
		{			
			case 1: 
			while (1) {
			printf("Esperando conexion...\n");
                    down(sem_servidor);
                    printf("Conexion realizada, creando hilo...\n");
			for (contador = 0; contador < 20; contador++) {
                        if (llaves_conexion[contador][1]) {
                            parametros_administrador[0] = llaves_conexion[contador][0];
                            parametros_administrador[1] = contador+200;
                            llaves_conexion[contador][1] = false;
                            break;
                        }
                    }

                    up(sem_administrador);
                    down(sem_servidor);

                    parametros_administrador[0] = 'x';
                    parametros_administrador[1] = 'x';

			administrador();	
			}
			break;
			
			case 2: 
					system("clear");
					printf("\n\n\t\t\t\t\t\t   1. Iniciar Sesion");
					printf("\n\n\t\t\t\t\t\t   2. Crear Cuenta");
	
					printf("\n\n\n\t\t\t\t     Ingresa la opcion correspondiente: ");
					scanf("%hd",&option); 

					switch (option)
					{
					case 1:
					sesion = iniciarSesion();
					if(sesion) //Si "iniciarSesion" retorna 0, entonces la cuenta o el nip son incorrectos.
						cliente(sesion);
						break;
					case 2:
					while (1) {
			printf("Esperando conexion...\n");
                    down(sem_servidor);
                    printf("Conexion realizada, creando hilo...\n");

                    for (contador = 0; contador < 20; contador++) {
                        if (llaves_conexion[contador][1]) {
                            parametros_cliente[0] = llaves_conexion[contador][0];
                            parametros_cliente[1] = contador+200;
                            llaves_conexion[contador][1] = false;
                            break;
                        }
                    }

                    up(sem_cliente);
                    down(sem_servidor);

                    parametros_cliente[0] = 'x';
                    parametros_cliente[1] = 'x';

					agregarCliente();

					printf("\n\n\t\t\t\t       Desea consultar su numero de cuenta ahora?");
					printf("\n\n\t\t\t\t\t\t   1. Si");
					printf("\n\n\t\t\t\t\t\t   2. No");
	
					printf("\n\n\n\t\t\t\t     Ingresa la opcion correspondiente: ");
					scanf("%hd",&option); 

					switch (option)
					{
					case 1:
						main();
						break;
					case 2:
					sesion = iniciarSesion();
					if(sesion) //Si "iniciarSesion" retorna 0, entonces la cuenta o el nip son incorrectos.
						cliente(sesion);
					break;
					}	
			}
						break;
					default:
					main();
						break;
					}
			break;
			case 0:
				
				printf("SISTEMA BANCARIO CERRADO \n\n");
				exit(0);

			break;
		
			default:
				printf("\n!!! Ha insertado una opcion invalida !!! Presione Enter para Intentar de Nuevo..."); 
				getchar(); getchar(); system("clear");
			break;		
		}

	} while (modo!=0);
	
	return 0;
}

void administrador()
{
	short int i, opcion = 1;//Se iniciliza "opcion" en 1 (true) para que entre por primera vez al while.
	int adminNip=2467, adminCuenta=16470179;
			
	while(opcion)
	{
		system("clear");

		
			printf("\n\n\t\t\t\t\tIngrese el numero de cuenta del Administrador: ");
			scanf("%d",&adminCuenta);
		
			printf("\n\n\t\t\t\t\tIngrese NIP: ");
			scanf("%d",&adminNip);

		if (adminCuenta!=16470179 || adminNip!=2467)
		{
			printf("\n\n\t\t\t\t\tIngrese los datos correctos");
			administrador();
		}
		

		setbuf(stdin,NULL);
		printf("\n\t\t\t\t\t A D M I N I S T R A D O R");
		printf("\n\t\t"); //Salto de linea y tabulador para mostrar la linea mas centrada.
		
		for(i=0; i<80; i++) //Ciclo para ir mostrando una linea de separacion.
			printf("*");
			
		printf("\n\n\t\t\t\t\t[1] Mostrar todos los clientes.");
		printf("\n\n\t\t\t\t\t[2] Buscar Cliente.");
		printf("\n\n\t\t\t\t\t[3] Ordenar clientes por nombre.");
		printf("\n\n\t\t\t\t\t[4] Editar datos de cuenta.");
		printf("\n\n\t\t\t\t\t[5] Eliminar cuenta.");
		printf("\n\n\t\t\t\t\t[0] Salir.");
		
		printf("\n\n\n\t\t\t\t\tIngrese opcion: ");
		scanf("%hd",&opcion);

		system("clear");
		switch(opcion)
		{	
			case 1: mostrarClientes();		break;
			
			case 2: buscarCliente();		break;
			
			case 3: ordenarClientesName();	break;
			
			case 4: editarDatos();			break;
			
			case 5: eliminarDatos();		
											break;
			case 0: main();
											break;
		}
	}
	
}

void agregarCliente(void) //Modulo para agregar cliente.
{
	FILE *f;
	clienteT usuario;
	time_t fechaActual;
	struct tm fecha; 
	short int cuentaExistente;
	unsigned int numeroCuenta;
						  
	f = fopen("clientes.dat","ab+"); //Abre el archivo en modo escritura/lectura (si no existe lo crea)
	                             
	if(!f)
	{
		printf("\n\n\t\t\t\t\t\aNo se ha podido abrir el archivo en modo \"ab+\".");
		system("pause>nul");
		exit(-1); //Salir del programa para evitar que se hagan operaciones con el archivo.
	}
	
	do //Bucle para comprobar que el numero de cuenta NO se repita.
	{
		cuentaExistente = 0; //Se asume que la cuenta NO va ser repetida.
		numeroCuenta = 100000 + rand() % (1000000 - 100000); //Li + rand() % ( (Ls+1) - Li).
		
		rewind(f); //Regresa el puntero del archivo al inicio, (el puntero es como el cursor).
		
		while(!feof(f) && !cuentaExistente)
		{
			fread(&usuario, sizeof(clienteT), 1, f); //Lee los registros 1 por 1.
			if(usuario.cuenta == numeroCuenta) //Verifica si ese numero de cuenta ya existe.
				cuentaExistente = 1; //En caso de existir, entonces variable cambia a 1 (verdadero).
		}
	
	}while(cuentaExistente); //Si variable cambia a verdadero, bucle se vuelve a repetir.
	
	usuario.cuenta = numeroCuenta; //Si sale del bucle, numero de cuenta generado NO existe y se almacena.
	
	setbuf(stdin,NULL);
	printf("\n\t\tIngresa el nombre del cliente (25 caracteres): ");
	fgets(usuario.nombre, T, stdin);
		
	printf("\n\t\tIngresa el apellido del cliente (25 caracteres): ");
	fgets(usuario.apellido, T, stdin);
		
	printf("\n\t\tIngresa el correo electronico del cliente (35 caracteres): ");	
	fgets(usuario.correo, T+10, stdin);

	printf("\n\t\tIngresa el numero de telefono del cliente: ");
	fgets(usuario.telefono, T-10, stdin);
		
	printf("\n\t\tIngresa la cantidad de fondos: ");
	scanf("%lf",&usuario.fondos);
	
	printf("\n\t\tIngresa el NIP (4 digitos): ");
	scanf("%4hd",&usuario.nip); 
	setbuf(stdin,NULL);
	
	fechaActual = time(NULL);
	fecha = *localtime(&fechaActual);
	
	//Se guarda la fecha de registro.
	usuario.fRe.dia = fecha.tm_mday;
	usuario.fRe.mes = fecha.tm_mon + 1;
	usuario.fRe.anio = fecha.tm_year;
	usuario.fRe.anio += 1900; //Se le suma 1900 porque "fecha.tm_year" empieza a contar desde 1900.
	
	//Se guarda la fecha en que se creo como ultimo movimiento.
	usuario.fMo.dia = fecha.tm_mday;
	usuario.fMo.mes = fecha.tm_mon + 1;
	usuario.fMo.anio = fecha.tm_year;
	usuario.fMo.anio += 1900;
		
	fwrite(&usuario, sizeof(clienteT), 1, f); //Se escribe en el fichero los datos del usuario.

	printf("\n\n\t\t\t\t\tSe ha guardado el cliente con exito.");
	system("clear");
	printf("\n\n\t\t\t\t\tPara mayor seguridad consulte su numero de cuenta con el administrador.");
	
	fclose(f);
	system("pause>nul");
}

void mostrarClientes()
{
	FILE *f;
	clienteT usuario;
	short int i;
	unsigned long int registros;
	
	
	f = fopen("clientes.dat","rb"); //Se abre el archivo en modo lectura.
	
	if(!f)
	{
		printf("\n\n\n\n\t\t\t\t\a   No se ha podido abrir el archivo \"clientes.dat\"");
		printf("\n\n\t\t\t1. Es posible que aun NO se haya registrado ninguna cuenta de banco.");
		printf("\n\n\t\t2. En caso de haberla creado, verifique tener el archivo en la misma carpeta del programa.");
		system("pause>nul");
		return; 
	}
	
	fseek(f, 0, SEEK_END); //Se va hasta el final del archivo.
	registros = ftell(f) / sizeof(clienteT); //Ftell regresa la cantidad de bytes leidos se divide entre
	//el tamano de "clientesT" para saber cuantos registros hay.
	rewind(f); //Regresar el puntero del archivo al inicio.
	
	for(i=0; i<registros; i++)
	{
		printf("\n\t\t\t\t\t\t   Cliente %hd",i+1);
		fread(&usuario, sizeof(clienteT), 1, f); //Lee un registro.
		printf("\n\n\t\t\t\t\tNumero de cuenta....... %u",usuario.cuenta);
		printf("\n\t\t\t\t\tNombre................. %s",usuario.nombre);		
		printf("\t\t\t\t\tApellidos.............. %s",usuario.apellido);
		printf("\t\t\t\t\tFecha de registro...... %hd / %hd / %hd",usuario.fRe.dia,usuario.fRe.mes,usuario.fRe.anio);		
		printf("\n\t\t\t\t\tNumero de telefono..... %s",usuario.telefono);
		printf("\t\t\t\t\tCorreo................. %s",usuario.correo);
		printf("\t\t\t\t\tFondos................. $ %G",usuario.fondos);
		printf("\n\t\t\t\t\tNIP.................... %hd",usuario.nip);		
		printf("\n\t\t\t\t\tultimo movimiento...... %hd / %hd / %hd\n\n",usuario.fMo.dia,usuario.fRe.mes,usuario.fRe.anio);
	}
	printf("\nPresione Enter para continuar..."); 
				getchar(); getchar(); system("clear");
	fclose(f);
	system("pause>nul");
}

void buscarCliente()
{
	FILE *f;
	clienteT usuario;
	short int i, modoBusqueda = 1; //Se inicializa en 1 (true) para entrar en el while por primera vez.
	unsigned int cuenta;
	unsigned long int posicion;
	char cadenaAux[T+10];
	
	f = fopen("clientes.dat","rb");	//Se abre archivo en modo lectura.

	if(!f) //Verifica que se abra correctamente.
	{
		printf("\n\n\n\n\t\t\t\t\a   No se ha podido abrir el archivo \"clientes.dat\"");
		printf("\n\n\t\t\t1. Es posible que aun NO se haya registrado ninguna cuenta de banco.");
		printf("\n\n\t\t2. En caso de haberla creado, verifique tener el archivo en la misma carpeta del programa.");
		system("pause>nul");
		return; //Salir del modulo para evitar que se hagan operaciones con el archivo.
	}
		
	while(modoBusqueda) //Bucle para que no se salga del menu hasta que usuario elija.
	{
		posicion = 0; 
		
		do
		{	
			setbuf(stdin,NULL);
			system("clear");
			
			printf("\n\t\t\t\t\tM O D O    D E    B U S Q U E D A");
			printf("\n\t\t"); //Salto de linea y tabulador para mostrar la linea mas centrada.
			
			for(i=0; i<80; i++) //Ciclo para ir mostrando una linea de separacion.
				printf("*");
			
			printf("\n\n\t\t\t\t\t  1) Numero de cuenta.");
			printf("\n\n\t\t\t\t\t  2) Numero telefonico.");
			printf("\n\n\t\t\t\t\t  3) Nombre del cliente.");
			printf("\n\n\t\t\t\t\t  4) Correo electronico.");
			printf("\n\n\t\t\t\t\t  0) Salir.");
			
			printf("\n\n\n\t\t\t\t\tIngresa el modo de busqueda: ");
			scanf("%hd",&modoBusqueda);
			
		}while(modoBusqueda < 0 || modoBusqueda > 4);
	
		setbuf(stdin,NULL); //Limpiar el buffer.
		
		if(!modoBusqueda)	
		{	
			fclose(f);
			return; //Si usuario presiona 0 (falso) la funcion termina.
		}
		
		else if(modoBusqueda == 1) 
		{
			printf("\n\n\t\t\t\t\tIngresa el numero de cuenta: ");
			scanf("%u",&cuenta);
		}
		
		else if(modoBusqueda == 2) //Su numero de telefono.
		{
			printf("\n\n\t\t\t\t\tIngresa el telefono del cliente: ");
			fgets(cadenaAux, T, stdin);
		}
	
		else if(modoBusqueda == 3)
		{
			printf("\n\n\t\t\t\t\tIngresa el nombre del cliente: ");
			fgets(cadenaAux, T, stdin);

		}
		
		else if(modoBusqueda == 4)
		{
			printf("\n\n\t\t\t\t\tIngresa el correo del cliente: ");
			fgets(cadenaAux, T+10, stdin);
		}	
		
		rewind(f); //El puntero regresa al inicio del archivo.
		
		while(!feof(f) && !posicion) //Termina hasta llegar al fin del archivo, o hasta encontrar el dato.
		{
			
			fread(&usuario, sizeof(clienteT), 1, f); //Lee registro.
			
			if(modoBusqueda == 1)
			{
				if(usuario.cuenta == cuenta)
					posicion = ftell(f); //Se guarda la posicion en la que haya terminado de leer.
			}
					
			else if(modoBusqueda == 2)
			{
				if( !strcmp(usuario.telefono,cadenaAux) ) 
					posicion = ftell(f);
			}

			else if(modoBusqueda == 3)
			{
				if( !strcmp(usuario.nombre,cadenaAux) )
					posicion = ftell(f);
			}
					
			else if(modoBusqueda == 4)
			{
				if( !strcmp(usuario.correo,cadenaAux) )
					posicion = ftell(f);
			}
					
		}
			
		if(!posicion) 
			printf("\n\n\t\t\t\t\tNo se ha encontrado ningun cliente.");
			
		else
		{
			system("clear");
			fseek(f, posicion-sizeof(clienteT), SEEK_SET); //Se regresa puntero del archivo al inicio
			//y se desplaza hasta la diferencia en bytes del segundo parametro.
			
			printf("\n\n\t\t\t\t\tNumero de cuenta....... %u",usuario.cuenta);
			printf("\n\t\t\t\t\tNombre................. %s",usuario.nombre);		
			printf("\t\t\t\t\tApellidos.............. %s",usuario.apellido);
			printf("\t\t\t\t\tFecha de registro...... %hd / %hd / %hd",usuario.fRe.dia,usuario.fRe.mes,usuario.fRe.anio);		
			printf("\n\t\t\t\t\tNumero de telefono..... %s",usuario.telefono);
			printf("\t\t\t\t\tCorreo................. %s",usuario.correo);
			printf("\t\t\t\t\tFondos................. $ %G",usuario.fondos);
			printf("\n\t\t\t\t\tNIP.................... %hd",usuario.nip);		
			printf("\n\t\t\t\t\tultimo movimiento...... %hd / %hd / %hd",usuario.fMo.dia,usuario.fRe.mes,usuario.fRe.anio);	
		}
		
		system("pause>nul");
	}
	printf("\nPresione Enter para continuar..."); 
				getchar(); getchar(); system("clear");
	fclose(f); //Cierra el archivo.
}

void ordenarClientesName()
{
	FILE *f;
	clienteT usuario, *usuarioAux = NULL;
	char nombre[T], aux[T];
	short int i, j; 
	unsigned long int registros;
	
	
	
	f = fopen("clientes.dat","rb"); //Abrir el archivo en modo lectura.
			
	if(!f)
	{
		printf("\n\n\n\n\t\t\t\t\a   No se ha podido abrir el archivo \"clientes.dat\"");
		printf("\n\n\t\t\t1. Es posible que aun NO se haya registrado ninguna cuenta de banco.");
		printf("\n\n\t\t2. En caso de haberla creado, verifique tener el archivo en la misma carpeta del programa.");
		system("pause>nul");
		return; 
	}
	
	fseek(f, 0, SEEK_END); //El puntero del archivo se va hasta el final.
	registros = ftell(f) / sizeof(clienteT); // guardar la cantidad de registros en el fichero.
	rewind(f); //Regresar el puntero del archivo al inicio.
	
	usuarioAux = (clienteT*) malloc( registros * sizeof(clienteT) ); //Se reserva memoria del heap.

	if(!usuarioAux) //Verifica que se haya podido reservar el espacio pedido.
	{
		printf("\n\t\t\tNo se ha podido reservar suficiente memoria.");
		system("pause>nul");
		exit(-1);
	}
	
	for(i=0; i<registros; i++) //Guarda los registros en la estructura dinamica.
		fread(usuarioAux, sizeof(clienteT), registros, f);

	rewind(f); //Regresar al principio del archivo.
	
	for(i=0; i<registros; i++) //Bucle para ordenar los nombres alfabeticamente.
		for(j=i; j<registros; j++) //Bucle para que el registro numero "i" se vaya comparando con los demas.
			if( strcmp( usuarioAux[i].nombre, usuarioAux[j].nombre) > 0 ) //Verifica el orden alfabetico.
			{//Si entra en el condicional, entonces el elemento "i" es MAYOR alfabeticamente.
				strcpy(aux, usuarioAux[i].nombre); //Se copia el elemento numero "i" en aux.
				strcpy(usuarioAux[i].nombre, usuarioAux[j].nombre);//Se copia el elemento numero "j" a "i".
				strcpy(usuarioAux[j].nombre, aux);//Ahora en "aux" se copia el elemento "i" en numero "J".
			}
	
	for(i=0; i<registros; i++)
	{
		rewind(f); //Regresar el puntero del archivo al inicio para volver a leer el fichero completo.
		for(j=0; j<registros; j++) 
		{
			fread(&usuario, sizeof(clienteT), 1, f);//Va leyendo cada registro del archivo.
			if( !strcmp(usuario.nombre, usuarioAux[i].nombre) ) //Verifica cual es el nombre que coincide.
			{ 
				printf("\n\t\t\t\t\tNumero de cuenta....... %u",usuario.cuenta);
				printf("\n\t\t\t\t\tNombre................. %s",usuario.nombre);		
				printf("\t\t\t\t\tApellidos.............. %s",usuario.apellido);
				printf("\t\t\t\t\tFecha de registro...... %hd / %hd / %hd",usuario.fRe.dia,usuario.fRe.mes,usuario.fRe.anio);		
				printf("\n\t\t\t\t\tNumero de telefono..... %s",usuario.telefono);
				printf("\t\t\t\t\tCorreo................. %s",usuario.correo);
				printf("\t\t\t\t\tFondos................. $ %G",usuario.fondos);
				printf("\n\t\t\t\t\tNIP.................... %hd",usuario.nip);		
				printf("\n\t\t\t\t\tultimo movimiento...... %hd / %hd / %hd\n\n",usuario.fMo.dia,usuario.fRe.mes,usuario.fRe.anio);			
			}			
		}
	}
		
	free(usuarioAux); //Liberar la memoria que se reserva.
	printf("\nPresione Enter para continuar..."); 
				getchar(); getchar(); system("clear");	
	fclose(f);//Cerrar el fichero.
	system("pause>nul");	
}

void editarDatos()
{
	FILE *f;
	clienteT usuario;
	short int encontrado = 0;
	unsigned int cuenta;
	unsigned long int posicion;
	
	f = fopen("clientes.dat","r+b"); //se abre en modo lectura y escritura.
	
	if(!f)
	{
		printf("\n\n\n\n\t\t\t\t\a   No se ha podido abrir el archivo \"clientes.dat\"");
		printf("\n\n\t\t\t1. Es posible que aun NO se haya registrado ninguna cuenta de banco.");
		printf("\n\n\t\t2. En caso de haberla creado, verifique tener el archivo en la misma carpeta del programa.");
		system("pause>nul");
		return; 
	}
	
	printf("\n\n\t\t\t\tIngrese el numero de cuenta: ");
	scanf("%u",&cuenta);
	setbuf(stdin,NULL);
	
	rewind(f);
	while(!feof(f) && !encontrado ) //Termina hasta leer todo el archivo, o hasta encontrar el num. cuenta.
	{
		fread(&usuario, sizeof(clienteT), 1, f);
		if(usuario.cuenta == cuenta) //Verifica si el numero de cuenta, es el mismo del registro.
			encontrado = 1; //Si coincide, entonces "encontrado" toma valor de 1 (verdadero).
	}
	
	if(!encontrado) //Si encontrado vale 0 (false), entonces ese numero de cuenta NO existe.
	{
		printf("\n\n\t\t\tEse numero de cuenta no se encuentra asociado a ningun cliente.");
		fclose(f); //Se cierra el archivo.
		system("pause>nul");
		return; 
	}
	
	posicion = ftell(f); //Se guarda la posicion en donde encontro el registro del numero de cuenta.

	while(encontrado) //Se usa la variable "encontrado" como auxliar.
	{
		system("clear");
		
		fseek(f, posicion-sizeof(clienteT), SEEK_SET); //Se regresa el puntero a la posicion 
		fread(&usuario, sizeof(clienteT), 1, f); //Guarda el nuevo dato, el puntero del archivo se desplaza.
	
		printf("\n\n\t\t\t\t\tNumero de cuenta....... %u",usuario.cuenta);
		printf("\n\t\t\t\t\tNombre................. %s",usuario.nombre);		
		printf("\t\t\t\t\tApellidos.............. %s",usuario.apellido);
		printf("\t\t\t\t\tFecha de registro...... %hd / %hd / %hd",usuario.fRe.dia,usuario.fRe.mes,usuario.fRe.anio);		
		printf("\n\t\t\t\t\tNumero de telefono..... %s",usuario.telefono);
		printf("\t\t\t\t\tCorreo................. %s",usuario.correo);
		printf("\t\t\t\t\tFondos................. $ %G",usuario.fondos);
		printf("\n\t\t\t\t\tNIP.................... %hd",usuario.nip);		
		printf("\n\t\t\t\t\tultimo movimiento...... %hd / %hd / %hd",usuario.fMo.dia,usuario.fRe.mes,usuario.fRe.anio);	
		
		printf("\n\n\n\t\t\t\t\t E D I T A R    D A T O S\n\t\t");
		for(encontrado=0; encontrado<80; encontrado++)//se usa "encontrado" como auxiliar para el for.
			printf("*"); //Imprime linea divisora.
			
		printf("\n\n\t\t\t\t\t\t1. Nombre.");
		printf("\n\t\t\t\t\t\t2. Apellidos.");
		printf("\n\t\t\t\t\t\t3. Numero de telefono.");
		printf("\n\t\t\t\t\t\t4. Correo electronico.");
		printf("\n\t\t\t\t\t\t5. Fondos.");
		printf("\n\t\t\t\t\t\t6. NIP.");
		printf("\n\t\t\t\t\t\t7. ultimo movimiento.");
		printf("\n\t\t\t\t\t\t0. Salir.");
		
		printf("\n\n\n\t\t\t\t\tIngresa el numero de dato a editar: ");
		scanf("%hd",&encontrado); //Se usa "encontrado" como variable auxiliar para almacenar la opcion.
	
		setbuf(stdin,NULL);
		if(encontrado>0 && encontrado<7)
			printf("\n\t\t\t\tIngresa el nuevo dato: ");
			
		switch(encontrado)
		{
			case 1: fgets(usuario.nombre, T, stdin); 		break;
			
			case 2: fgets(usuario.apellido, T, stdin);		break;
			
			case 3: fgets(usuario.telefono, T-10, stdin);	break;
			
			case 4: fgets(usuario.correo, T+10, stdin);		break;
			
			case 5: scanf("%lf",&usuario.fondos);			break;
			
			case 6:	scanf("%4hd",&usuario.nip);	setbuf(stdin,NULL);	break;
			
			case 7: printf("\n\n\t\tIngrese la nueva fecha en formato dia/mes/ano ejemplo (11/11/2021): ");
					scanf("%hd/%hd/%hd",&usuario.fMo.dia,&usuario.fMo.mes,&usuario.fMo.anio); 
		}
		
		if(encontrado>0 && encontrado<8) //Entra al condicional solo si presiono el rango de esos numeros.
		{
			fseek(f, posicion-sizeof(clienteT), SEEK_SET); //Se regresa el puntero a la posicion anterior.
			fwrite(&usuario, sizeof(clienteT), 1, f); //Guarda el nuevo dato, el puntero del archivo se desplaza.
		}
	}
	printf("\nPresione Enter para continuar..."); 
				getchar(); getchar(); system("clear");		
	fclose(f); //Se cierra el fichero.
}

void eliminarDatos()
{
	FILE *f, *fAux;
	clienteT usuario, usuarioAux;
	short int encontrado = 0;
	unsigned int cuenta;
	unsigned long int registros, posicion;
	
	
	f = fopen("clientes.dat", "rb"); //Se abre en modo lectura
	
	if(!f)
	{
		printf("\n\n\n\n\t\t\t\t\a   No se ha podido abrir el archivo \"clientes.dat\"");
		printf("\n\n\t\t\t1. Es posible que aun NO se haya registrado ninguna cuenta de banco.");
		printf("\n\n\t\t2. En caso de haberla creado, verifique tener el archivo en la misma carpeta del programa.");
		system("pause>nul");
		return; 
	}
	
	printf("\n\n\t\t\t\t     Ingrese el numero de cuenta: ");
	scanf("%u",&cuenta);
	setbuf(stdin,NULL);
	
	rewind(f); //Regresar el puntero del archivo al inicio.
	while(!feof(f) && !encontrado ) //Termina hasta leer todo el archivo, o hasta encontrar el num. cuenta.
	{
		fread(&usuario, sizeof(clienteT), 1, f);
		if(usuario.cuenta == cuenta) //Verifica si el numero de cuenta, es el mismo del registro.
			encontrado = 1; //Si coincide, entonces "encontrado" toma valor de 1 (verdadero).
	}
	
	if(!encontrado) //Si encontrado vale 0 (false), entonces ese numero de cuenta NO existe.
	{
		printf("\n\n\t\t\tEse numero de cuenta no se encuentra asociado a ningun cliente.");
		fclose(f); //Se cierra el archivo.
		system("pause>nul");
		return; 
	}
	
	printf("\n\n\t\t\t\t\tNumero de cuenta....... %u",usuario.cuenta);
	printf("\n\t\t\t\t\tNombre................. %s",usuario.nombre);		
	printf("\t\t\t\t\tApellidos.............. %s",usuario.apellido);
	printf("\t\t\t\t\tFecha de registro...... %hd / %hd / %hd",usuario.fRe.dia,usuario.fRe.mes,usuario.fRe.anio);		
	printf("\n\t\t\t\t\tNumero de telefono..... %s",usuario.telefono);
	printf("\t\t\t\t\tCorreo................. %s",usuario.correo);
	printf("\t\t\t\t\tFondos................. $ %G",usuario.fondos);
	printf("\n\t\t\t\t\tNIP.................... %hd",usuario.nip);		
	printf("\n\t\t\t\t\tultimo movimiento...... %hd / %hd / %hd",usuario.fMo.dia,usuario.fRe.mes,usuario.fRe.anio);	
	
	printf("\n\n\n\t\t\t\t¿Estas seguro que deseas eliminar esta cuenta?");
	printf("\n\n\t\t\t\t\t\t   1. Si");
	printf("\n\n\t\t\t\t\t\t   2. No");
	
	printf("\n\n\n\t\t\t\t     Ingresa la opcion correspondiente: ");
	scanf("%hd",&encontrado); //Se usa variable "encontrado" como auxiliar.
	
	if(encontrado != 1)
	{
		fclose(f); //Se cierra el archivo.
		return; 		
	}

	fAux = fopen("clientesAux.dat","wb"); //Abre el archivo en modo escritura (si no existe, lo crea).
	
	if(!fAux)
	{
		printf("\n\n\n\n\t\t\t\t\a   No se ha podido crear el archivo \"aux.dat\"");
		fclose(f); //Cerrar el archivo "clientes.dat"
		system("pause>nul");
		return; 
	}
	
	rewind(f); //Regresar el puntero del archivo al inicio.
	fread(&usuarioAux, sizeof(clienteT), 1, f); //Lee el primer registros.
	while( !feof(f) ) //Termina hasta llegara al final del archivo.
	{
		if(usuarioAux.cuenta != cuenta) //Verifica que la cuenta NO coincida con el registro leido.
			fwrite(&usuarioAux, sizeof(clienteT), 1, fAux); //Si NO coincide, se copia al fichero auxiliar.
		
		fread(&usuarioAux, sizeof(clienteT), 1, f); //Lee los registros.
	}
	
	printf("\n\n\n\t\t\t\t\t    La cuenta ha sido eliminada.");

	printf("\nPresione Enter para continuar..."); 
				getchar(); getchar(); system("clear");
	fclose(f); //Cerrar ficheros.
	fclose(fAux);
	remove("clientes.dat"); //Eliminar el ficher "clientes.dat" (contiene TODA la informacion).
	rename("clientesAux.dat","clientes.dat"); //Renombrar al nuevo fichero. (toda la info. menos la eliminada).
	system("pause>nul");
}

unsigned int iniciarSesion()
{
	FILE *f;
	clienteT usuario;
	short int nip, encontrado = 0;
	unsigned int cuenta;

	

	f = fopen("clientes.dat","rb"); //Se abre archivo en modo lectura.
	
	if(!f) //Verifica que se haya abierto correctamente.
	{
		printf("\n\n\t\t\t\t\t\aNo se ha creado ninguna cuenta.");
		system("pause>nul");
		return 0; 
	}
	
	system("clear");
	printf("\n\n\t\t\t\t\tIngrese el numero de cuenta: ");
	scanf("%u",&cuenta);
	
	printf("\n\n\t\t\t\t\tIngrese NIP: ");
	scanf("%hd",&nip);
	
	while(!feof(f) && !encontrado) //Bucle para buscar la cuenta y el nip.
	{
		fread(&usuario, sizeof(clienteT), 1, f);
		if( usuario.cuenta == cuenta && usuario.nip == nip)
			encontrado = 1;
	}

	if(!encontrado) //Si encontrado sigue valiendo 0 (falso) entonces NO encunetro la cuenta.
	{
		printf("\n\n\t\t\t\t\tLos datos ingresados son incorrectos.");
		fclose(f);
		system("pause>nul");
		return 0; //Regresa 0 (falso)
	}
	
	fclose(f); //Se cierra el archivo.
	return usuario.cuenta; //Regresa el numero de cuenta.
}

void cliente(unsigned int cuenta)
{
	short int i, opcion = 1;
	
	while(opcion)
	{
		system("clear");
		setbuf(stdin,NULL);
		
		printf("\n\t\t\t\t\t     C L I E N T E");
		printf("\n\t\t"); //Salto de linea y tabulador para mostrar la linea mas centrada.
		
		for(i=0; i<80; i++) //Ciclo para ir mostrando una linea de separacion.
			printf("*");
			
		printf("\n\n\t\t\t\t\t[1] Consultar informacion.");
		printf("\n\n\t\t\t\t\t[2] Realizar deposito.");
		printf("\n\n\t\t\t\t\t[3] Realizar retiro.");
		printf("\n\n\t\t\t\t\t[4] Realizar transferencia.");
		printf("\n\n\t\t\t\t\t[0] Salir.");
		
		printf("\n\n\n\t\t\t\t\tIngrese opcion: ");
		scanf("%hd",&opcion);

		system("clear");
		switch(opcion)
		{
			case 1:	mostrarInformacion(cuenta);	break; 
			
			case 2: realizarDeposito(cuenta);	break;
			
			case 3: realizarRetiro(cuenta);		break;
			
			case 4: transferir(cuenta);			break;
		}
	}
	
}

void mostrarInformacion(unsigned int cuenta)
{
	FILE *f;
	clienteT usuario;
	short int encontrado = 0;
	
	f = fopen("clientes.dat","rb"); //Se abre en modo lectura.
	
	if(!f)
	{
		printf("\n\n\n\n\t\t\t\t\a   No se ha podido abrir el archivo \"clientes.dat\"");
		fclose(f);
		system("pause>nul");
		exit(-1); 
	}
	
	while(!feof(f) && !encontrado)
	{
		fread(&usuario, sizeof(clienteT), 1, f);
		if( usuario.cuenta == cuenta ) //Busca el registro asoaciado a la cuenta.
			encontrado = 1;	
	}
	
	printf("\n\n\t\t\t\t\tNumero de cuenta....... %u",usuario.cuenta);
	printf("\n\t\t\t\t\tNombre................. %s",usuario.nombre);		
	printf("\t\t\t\t\tApellidos.............. %s",usuario.apellido);
	printf("\t\t\t\t\tFecha de registro...... %hd / %hd / %hd",usuario.fRe.dia,usuario.fRe.mes,usuario.fRe.anio);		
	printf("\n\t\t\t\t\tNumero de telefono..... %s",usuario.telefono);
	printf("\t\t\t\t\tCorreo................. %s",usuario.correo);
	printf("\t\t\t\t\tFondos................. $ %G",usuario.fondos);
	printf("\n\t\t\t\t\tNIP.................... %hd",usuario.nip);		
	printf("\n\t\t\t\t\tultimo movimiento...... %hd / %hd / %hd\n\n",usuario.fMo.dia,usuario.fRe.mes,usuario.fRe.anio);

	printf("\nPresione Enter para continuar..."); 
				getchar(); getchar(); system("clear");
	fclose(f);
	system("pause>nul");
}

void realizarDeposito(unsigned int cuenta)
{
	FILE *f;
	clienteT usuario;
	short int encontrado = 0;
	double deposito;
	unsigned long int posicion;
	
	f = fopen("clientes.dat", "rb+"); //Se abre en modo lectura y escritura.
	
	if(!f)
	{
		printf("\n\n\n\n\t\t\t\t\a   No se ha podido abrir el archivo \"clientes.dat\"");
		fclose(f);
		system("pause>nul");
		exit(-1); 
	}	
	
	printf("\n\n\t\t\t\t\tIngrese el monto deposito: ");
	scanf("%lf",&deposito);
	
	if(deposito <= 0) //Condicional para verificar que la cantidad a depositar sea mayor a 0.
	{
		printf("\n\n\t\t\t\tDebes de ingresar una cantidad mayor a 0.");
		fclose(f); //Se cierra el archivo.
		system("pause>nul");
		return; 
	}
	
	while(!feof(f) && !encontrado) //Bucle para llegar al registro de la cuenta selecionada.
	{
		fread(&usuario, sizeof(clienteT), 1, f);
		if( usuario.cuenta == cuenta)
		{	
			encontrado = 1;
			posicion = ftell(f); //Se guarda la posicion.
		}
	}

	usuario.fondos += deposito; //Se suma lo depositado a la cuenta.
	fseek(f, posicion-sizeof(clienteT), SEEK_SET); //Se desplaza hacia el registro de la cuenta.
	fwrite(&usuario, sizeof(clienteT), 1, f); //Se guarda el el registro.
	
	printf("\n\n\t\t\t\t\tSe ha hecho el deposito a tu cuenta.");	
		
	printf("\nPresione Enter para continuar..."); 
				getchar(); getchar(); system("clear");
	fclose(f);
	system("pause>nul");
}

void realizarRetiro(unsigned int cuenta)
{
	FILE *f;
	clienteT usuario;
	short int encontrado = 0;
	double retiro;
	unsigned long int posicion;
	
	f = fopen("clientes.dat","rb+");
	
	if(!f)
	{
		printf("\n\n\n\n\t\t\t\t\a   No se ha podido abrir el archivo \"clientes.dat\"");
		fclose(f);
		system("pause>nul");
		exit(-1); 
	}

	printf("\n\n\t\t\t\t\tIngresa el monto del retiro: ");
	scanf("%lf",&retiro);
	
	if(retiro <= 0) //Condicional para verificar que la cantidad a retirar sea mayor a 0.
	{
		printf("\n\n\t\t\t\t\tDebes de ingresar una cantidad mayor a 0.");
		fclose(f); //Se cierra el archivo.
		system("pause>nul");
		return; 
	}

	while(!feof(f) && !encontrado) //Bucle para llegar al registro de la cuenta selecionada.
	{
		fread(&usuario, sizeof(clienteT), 1, f);
		if( usuario.cuenta == cuenta)
		{	
			encontrado = 1;
			posicion = ftell(f); //Se guarda la posicion.
		}
	}

	if( usuario.fondos < retiro )
	{
		printf("\n\n\t\t\t\tNo dispones con los fondos suficientes para retirar.");
		fclose(f);
		system("pause>nul");
		return;
	}
	
	usuario.fondos -= retiro;
	fseek(f, posicion-sizeof(clienteT), SEEK_SET); //Se desplaza hacia el registro de la cuenta.
	fwrite(&usuario, sizeof(clienteT), 1, f); //Se guarda el el registro.
	
	printf("\n\n\t\t\t\t\tSe ha hecho el retiro de tu cuenta.");
	printf("\nPresione Enter para continuar..."); 
				getchar(); getchar(); system("clear");
	fclose(f);
	system("pause>nul");
	
}

void transferir(unsigned int cuenta)
{
	FILE *f;
	clienteT usuario;
	short int i, encontrado = 0, salir, intentar;
	unsigned int cuentaTrans;
	unsigned long int posicion, posicionAux;
	double monto;
	char nombre[T];
	
	f = fopen("clientes.dat","rb+");
	
	if(!f)
	{
		printf("\n\n\n\n\t\t\t\t\a   No se ha podido abrir el archivo \"clientes.dat\"");
		fclose(f); //Cerrar el archivo
		system("pause>nul");
		exit(-1); 
	}
	
	printf("\n\n\t\t\t\t\tIngrese el monto a transferir: ");
	scanf("%lf",&monto);
	
	if(monto <= 0) //Condicional para verificar que la cantidad a retirar sea mayor a 0.
	{
		printf("\n\n\t\t\t\t\tDebes de ingresar una cantidad mayor a 0.");
		fclose(f); //Se cierra el archivo.
		system("pause>nul");
		return; 
	}

	while(!feof(f) && !encontrado) //Bucle para llegar al registro de la cuenta selecionada.
	{
		fread(&usuario, sizeof(clienteT), 1, f);
		if( usuario.cuenta == cuenta)
		{	
			encontrado = 1;
			posicion = ftell(f); //Se guarda la posicion.
		}
	}

	if( usuario.fondos < monto )
	{
		printf("\n\n\t\t\t\tNo dispones con los fondos suficientes para retirar.");
		fclose(f);
		system("pause>nul");
		return;
	}
	
	do
	{
		setbuf(stdin,NULL);
		system("clear");
		
		printf("\n\n\t\t\t\tIngrese el numero de cuenta a transferir: ");
		scanf("%u",&cuentaTrans);
		
		encontrado = 0; //0 NO se encontro, 1 Si se encontro, 2 es la misma cuenta del enviador.
		salir = 1; //1 se repite, 0 sale.
		intentar = 1; 
		rewind(f); //Regresa el puntero del archivo al incio.
			
		while(!feof(f) && !encontrado) //Bucle para buscar la cuenta a transferir.
		{
			fread(&usuario, sizeof(clienteT), 1, f);
			if(usuario.cuenta == cuentaTrans)
			{	
				if(cuenta == cuentaTrans)
					encontrado = 2;		
								
				else 
				{
					posicionAux = ftell(f);
					encontrado = 1;
					salir = 0;
				}
			}
		}
		
		if(!encontrado) //Si no se encuentra la cuenta, entra en el condicional.
		{
			printf("\n\n\t\t\t\t     No se ha encontrado ese numero de cuenta.");
			printf("\n\n\n\t\t\t\t\t\t1. Intentar de nuevo.");
			printf("\n\n\t\t\t\t\t\t0. Salir.");
			
			printf("\n\n\n\n\t\t\t\t\t\tIngrese opcion: ");
			scanf("%hd",&intentar);
			
			if(!intentar)
				salir = 0;
		}
		
		else if(encontrado == 2)
		{
			printf("\n\n\t\t\t\tNo puedes hacer la transferencia a tu propia cuenta.");
			printf("\n\n\n\t\t\t\t\t\t1. Intentar de nuevo.");
			printf("\n\n\t\t\t\t\t\t0. Salir.");
			
			printf("\n\n\n\n\t\t\t\t\t\tIngrese opcion: ");
			scanf("%hd",&intentar);
			
			if(!intentar)
				salir = 0;
		}
		
	}while(salir);
	
	if(!intentar) //Si usuario dio en salir (0), entonces entra en el condicional.
	{
		fclose(f); //Se cierra el archivo.
		return; 
	}
	
	for(i=0; usuario.nombre[i] != '\n'; i++) //Ciclo para quitarle el '\n' de "usuario.nombre".
		nombre[i] = usuario.nombre[i];
		
	nombre[i] = '\0'; //Al final pasarle el caracter nulo.
	
	printf("\n\n\t\t\t\t\tNumero de cuenta....... %u",usuario.cuenta);
	printf("\n\t\t\t\t\tNombre................. %s %s",nombre,usuario.apellido);		
	printf("\t\t\t\t\tNumero de telefono..... %s",usuario.telefono);
	printf("\t\t\t\t\tCorreo................. %s",usuario.correo);
	printf("\t\t\t\t\tMonto a transferir..... %G",monto);
	
	printf("\n\n\n\t\t\t\t\t\t0. Cancelar.");
	printf("\n\n\t\t\t\t\t\t1. Transferir.");
	
	printf("\n\n\n\t\t\t\t\tIngrese la opcion correspondiente: ");
	scanf("%hd",&intentar); //Se toma variable "intentar" como auxiliar.
	
	if(!intentar)
	{
		fclose(f); //Se cierra el archivo.
		return; 
	}

	fseek(f, posicion-sizeof(clienteT), SEEK_SET); //Puntero del archivo se desplaza al usuario que
	fread(&usuario, sizeof(clienteT), 1, f); //realizar la transferencia.
	
	usuario.fondos -= monto; //Se le resta el dinero transferido.
	
	fseek(f, posicion-sizeof(clienteT), SEEK_SET); //Se vuelve a desplazar el puntero del archivo a la
	fwrite(&usuario, sizeof(clienteT), 1, f); //posicion del usuario y se actualiza el registro.
	
	
	fseek(f, posicionAux-sizeof(clienteT), SEEK_SET); //Puntero del archivo se desplaza al usuario 
	fread(&usuario, sizeof(clienteT), 1, f); //a quien le van a transferir.
	
	usuario.fondos += monto; //Se le suma el dinero transferido.
	
	fseek(f, posicionAux-sizeof(clienteT), SEEK_SET); //Se vuelve a desplazar el puntero del archivo
	fwrite(&usuario, sizeof(clienteT), 1, f); //para actualizar el registro.
	
	printf("\n\n\n\n\t\t\t\t\t    Se ha hecho la trasnferencia.");

	printf("\nPresione Enter para continuar..."); 
				getchar(); getchar(); system("clear");
	fclose(f);
	system("pause>nul");
}
