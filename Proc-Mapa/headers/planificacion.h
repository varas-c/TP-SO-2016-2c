/*
 * planificacion.h
 *
 *  Created on: 18/9/2016
 *      Author: utnso
 */

#ifndef HEADERS_PLANIFICACION_H_
#define HEADERS_PLANIFICACION_H_
//***********************************************************************************

/*FUNCION PRINCIPAL!
- Invocar sort_SRDF para ordenar colaListos
- ColaListos es una VARIABLE GLOBAL
-
- NOTA: USAR SEMAFOROS! La funcion no tiene ninguno, implementarlos fuera de la función.
*/
//***********************************************************************************

/* FUNCIONES AUXILIARES A sort_SRDF() - NO INVOCAR PARA HACER EL ORDENAMIENTO - sort_SRDF YA LO HACE! */

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
			if(jugador1->numero < jugador2->numero) return 1; //Si el ingreso es menor es porque ENTRO PRIMERO
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
//*******************

Jugador* removeJugadorSocket(int socket)
{

	bool _find_Player_Socket(Jugador* jugador)
	{
		return jugador->socket == socket;
	}

	Jugador* jugadorEncontrado = (Jugador*) list_remove_by_condition(listaListos,(void*)_find_Player_Socket);
	return jugadorEncontrado;
}

Jugador* get_SRDF()
{
	int tamColaListos = list_size(listaListos);
	t_list* listaAuxiliar;
	Jugador* jugadorAux;
	Jugador* jugadorBuscado;

	listaAuxiliar = list_take(listaListos,tamColaListos);
	ordenarListaSRDF(listaAuxiliar);

	jugadorAux = list_get(listaAuxiliar,0);

	jugadorBuscado = removeJugadorSocket(jugadorAux->socket);

	list_destroy(listaAuxiliar);

	return jugadorBuscado;
}

#endif /* HEADERS_PLANIFICACION_H_ */
