#include <stdio.h>
#include <stdlib.h>
#include "../include/carrito.h"

// %%% Funciones %%%
carrito get ( int pos , lista l ) {
	carrito copia;
	linkList Ite= ubicar (pos , l);
	if (Ite == NULL)
	    return copia;
	copia=Ite->Inf;
	return copia;   
}                                        

linkList ubicar ( int p , lista l ){
	int mitad,i;
	linkList Ite;
	if (l == NULL || empty (l))
	    return NULL;    
	if ( p == 0 )    
	    return l->I;
	if ( p == l->NE-1 )    
	    return l->F;    
	if (p>0 && p<l->NE-1)    
	    {
	       mitad=l->NE/2;
	       if ( p >= mitad )
	        {
	        	Ite=l->I;
	        	for(i=0;i<p;i++)
	        	    Ite=Ite->sig;	  
			} else {
				Ite=l->F;	
	        	for(i=0;i<(l->NE-1)-p;i++)
	        	    Ite=Ite->ant;	    
			}	       
	       return Ite;
		}
	return NULL;	
} 

mensaje add ( int pos, carrito E , lista l ){
	linkList act,ant;
	linkList temp = (linkList) malloc (sizeof(NODO));
	if (temp == NULL)
	    return NO_MEMORY; 
	temp->Inf=E;
	if ( empty (l) )
	   {
	   	  temp-> ant = temp -> sig = NULL;
		  l->I=l->F=temp;	    
	   } else {
	   	  if (pos <= 0){ // Al Inicio
	   	  	    temp->sig=l->I;
	   	  	    temp->ant=NULL;
	   	  	    l->I->ant=temp;
	   	  	    l->I=temp;
			 }else if( pos >= l->NE){ // Al final
			 	temp->sig=NULL;
	   	  	    temp->ant=l->F;
	   	  	    l->F->sig=temp;
			    l->F=temp;
			 }else{ // En medio
		        act= ubicar (pos , l);
				ant= act->ant;
			 	temp->sig=act;
			 	temp->ant=ant;
			 	act->ant=temp;
			 	ant->sig=temp;	 	
			 }
	   }
	l->NE++;
	return OK;
}        

mensaje borrar (  int p , lista l ){
    linkList temp,s,a,act;
	if (l == NULL || empty (l))
	    return EMPTY;
	if ( !(p>=0 && p<l->NE) )    
		return INDEXOUTOFBOUND;
    if( p == 0 && l->NE == 1 ) // lista con un elemento
    {
    	 free(l->I);
    	 l->I=l->F=NULL;
    	 
	}else if (p == 0 && l->NE > 1){  // Al Inicio
		 temp=l->I->sig;
		 free(l->I);
		 temp->ant=NULL;
		 l->I=temp;
		 
	} else if ( p == l->NE-1 ){ // Al final
		  temp=l->F->ant;
		  free(l->F);
		  temp->sig=NULL;
		  l->F=temp;
		
	}else {  //En medio	
		act= ubicar (p , l);
		a=act->ant;
		s=act->sig;
		free(act);
		a->sig=s;
		s->ant=a;
	}
    l->NE--;
	return OK;   
}                

booleano empty ( lista l  ){
    if ( l != NULL && l->I == NULL && l->F == NULL && l->NE == 0 )	
	    return TRUE;
	return FALSE;
}

mensaje set ( int p , carrito E , lista l){
	linkList Ite;
	if (l == NULL || empty (l))
	    return EMPTY;
	if ( !(p>=0 && p<l->NE) )    
		return INDEXOUTOFBOUND;
	Ite=ubicar (p , l);	
	Ite->Inf=E;
	return OK;
}      

mensaje crearlista( lista * l ){
	lista temp= (lista) malloc (sizeof(cabecera));
	if ( temp == NULL)
	    return NO_MEMORY;
	temp->I=temp->F=NULL;
	temp->NE=0;
	*l=temp;
	return OK;
}

void  liberarlista( lista * l ){
	if ( *l != NULL ){
	  vaciarlista ( *l  );
	  free(*l);
	  *l=NULL;
    }
}  

void  vaciarlista ( lista l ){
	while (!empty (l)){
	   	borrar(0,l);   	
	}
}   