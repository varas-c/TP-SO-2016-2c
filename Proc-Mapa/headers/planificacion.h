/*
 * planificacion.h
 *
 *  Created on: 18/9/2016
 *      Author: utnso
 */

#ifndef HEADERS_PLANIFICACION_H_
#define HEADERS_PLANIFICACION_H_

//***********************************************************************************
//***********************************************************************************
//***********************************************************************************

/*FUNCION PRINCIPAL!
- Invocar sort_SRDF para ordenar colaListos
- ColaListos es una VARIABLE GLOBAL
-
- NOTA: USAR SEMAFOROS! La funcion no tiene ninguno, implementarlos fuera de la función.
*/
void sort_SRDF()
{
	t_list* lista = list_create(); //Creamos cola auxiliar.
	Jugador* aux;
	while(!queue_is_empty(colaListos))
	{
		aux = queue_pop(colaListos);
		list_add(lista,aux);
	}

	ordenarListaSRDF(lista);
	copiarListaCola(lista);
}



//***********************************************************************************
//***********************************************************************************
//***********************************************************************************


/* FUNCIONES AUXILIARES A sort_SRDF() - NO INVOCAR PARA HACER EL ORDENAMIENTO - sort_SRDF YA LO HACE! */

//***********************************************************************************
//***********************************************************************************
//***********************************************************************************


//Dado un entrenador, cacula cuanto le falta para llegar a destino
int movRestantes(Entrenador entrenador)
{
	int movRestantes;
	movRestantes = entrenador.destinox + entrenador.destinoy;
	return movRestantes;
}



//***********************************************************************************
/* Ordena una lista por SRDF
 * NOTA: ORDENA UNA LISTA, NO UNA COLA !!!!!
 */
void ordenarListaSRDF(t_list* lista)
{
	bool _less_distance_(Jugador* jugador1, Jugador* jugador2)
	{
		if(movRestantes(jugador1->entrenador) == movRestantes(jugador2->entrenador)) //SI SON IGUALES, DESEMPATAN POR FIFO
		{
			if(jugador1->ingreso < jugador2->ingreso) return 1; //Si el ingreso es menor es porque ENTRO PRIMERO
			else return 0;
		}

		if(movRestantes(jugador1->entrenador) < movRestantes(jugador2->entrenador)) return 1; //Si al jugador1 le falta MENOS, va primero!
		else return 0;
	}

	list_sort(lista,(void*)_less_distance_);
}


//***********************************************************************************
//Imprime una lista de Jugadores por pantalla.
void printf_lista(t_list* lista)
{
	int tam = list_size(lista);
	Jugador* jugador;
	int i;
	int movs;

	for(i=0;i<tam;i++)
	{
		jugador = list_get(lista,i);
		movs = movRestantes(jugador->entrenador);
		printf("Jugador: %i \n", movs);
	}
}


//***********************************************************************************
//Vuelca la Lista a una cola
void copiarListaCola(t_list* lista)
{
	int tam = list_size(lista);
	Jugador* jugador;
	int i;
	int movs;

	for(i=0;i<tam;i++)
	{
		jugador = list_get(lista,i);
		queue_push(colaListos,jugador);
	}

}





#endif /* HEADERS_PLANIFICACION_H_ */
