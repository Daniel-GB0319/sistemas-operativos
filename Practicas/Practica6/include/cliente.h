#ifndef _CARRITO_H_
#define _CARRITO_H_

typedef struct Cuenta_Cliente {
	unsigned long numCuenta; // Numero de cuenta
	char name[31]; // Nombre del cliente
	int nip; // Nip de la tarjeta
	int saldo; // Saldo de la cuenta	
} cuenta;

typedef struct nodo {
    cuenta Inf;
    struct nodo * sig;
	struct nodo * ant;  	
} NODO;

typedef struct Cab {
	NODO * I;
	NODO * F;
	int NE;
} cabecera;

typedef  cabecera * lista;
typedef  NODO * linkList;
typedef  enum men { NO_MEMORY , OK , INDEXOUTOFBOUND, EMPTY } mensaje;
typedef  enum b { FALSE , TRUE } booleano;

//Prototipos 
cuenta get ( int pos , lista l );  
linkList ubicar ( int pos , lista l );                                        
mensaje add ( int pos, cuenta E , lista l );         
mensaje borrar (  int Pos , lista l );                   
booleano empty ( lista l  );
mensaje set ( int Pos ,  cuenta E , lista l);      
mensaje  crearlista( lista * l ); 
void  liberarlista( lista * l );  	   
void vaciarlista( lista l ); 


#include "../src/cliente.c"

#endif 
