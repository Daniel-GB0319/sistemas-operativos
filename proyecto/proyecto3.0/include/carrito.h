#ifndef _CARRITO_H_
#define _CARRITO_H_

typedef struct Carrito_Compras {
	char name[21]; // Nombre del Producto
	int cant; // Cantidad del Producto
	int precio; // Precio del producto		
} carrito;

typedef struct nodo {
    carrito Inf;
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
carrito get ( int pos , lista l );  
linkList ubicar ( int pos , lista l );                                        
mensaje add ( int pos, carrito E , lista l );         
mensaje borrar (  int Pos , lista l );                   
booleano empty ( lista l  );
mensaje set ( int Pos ,  carrito E , lista l);      
mensaje  crearlista( lista * l ); 
void  liberarlista( lista * l );  	   
void vaciarlista( lista l ); 


#include "../src/carrito.c"

#endif 
