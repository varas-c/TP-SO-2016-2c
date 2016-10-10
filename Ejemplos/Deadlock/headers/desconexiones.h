/*
 * desconexiones.h
 *
 *  Created on: 6/10/2016
 *      Author: utnso
 */

#ifndef HEADERS_DESCONEXIONES_H_
#define HEADERS_DESCONEXIONES_H_

#include "headers/socket.h"
#include <errno.h>


void desconectarJugador(Jugador* jugador)
{
	BorrarItem(gui_items,jugador->entrenador.simbolo);
	FD_CLR(jugador->socket,&fds_entrenadores);
	close(jugador->socket);
	free(jugador);
}


void detectarDesconexiones()
{
	Jugador* jugador;
	int tamLista = list_size(colaListos);
	void* buffer;
	buffer = malloc(sizeof(int));

	int i=0;
	for(i=0;i<tamLista;i++)
	{
		jugador = (Jugador*) list_get(colaListos,i);

		if(recv(jugador->socket,buffer,sizeof(int),MSG_PEEK) == 0)
		{
			desconectarJugador(jugador);
		}
	}

	free(buffer);

}

void detectarDesconexiones2(){		//LA REVANCHA
	Jugador* jugador;
	int tamLista = list_size(colaListos);
	int i=0;
	int error = 0;
	socklen_t len = sizeof(error);
	for(i=0;i<tamLista;i++){
		error = 0;
		pthread_mutex_lock(&mutex_Listos);
		jugador = (Jugador*) list_get(colaListos,i);
		pthread_mutex_unlock(&mutex_Listos);
		int retval = getsockopt(jugador->socket, SOL_SOCKET, SO_ERROR, &error, &len);

		if(retval != 0){ //No detecta desconexiones
			jugador = list_remove(colaListos,i);
			desconectarJugador(jugador);
		}

		if(error != 0)
		{
			jugador = list_remove(colaListos,i);
			desconectarJugador(jugador);
		}
	}
}

void detectarDesconexiones3()	//LA VENGANZA
{
	int* socketDesconectar = NULL;
	int tamCola = queue_size(colaDesconectados);
	int i;
	Jugador* jugadorDesconectar = NULL;

	for(i=0;i<tamCola;i++)
	{
		pthread_mutex_lock(&mutex_Desconectados);
		socketDesconectar = queue_pop(colaDesconectados);
		pthread_mutex_unlock(&mutex_Desconectados);

		jugadorDesconectar = getRemove_JugadorPorSocket(*socketDesconectar);

		if(jugadorDesconectar != NULL)
		{
		desconectarJugador(jugadorDesconectar);
		}
		free(socketDesconectar);

	}

}

void detectarDesconexiones4()	//EL ULTIMATUM
{
	Jugador* jugador;
		int tamLista = list_size(colaListos);
		int i=0;
		void* buffer;
		struct sockaddr mysock;
		socklen_t addrlen = sizeof(mysock);

		buffer = malloc(sizeof(int));
		for(i=0;i<tamLista;i++){
			pthread_mutex_lock(&mutex_Listos);
			jugador = (Jugador*) list_get(colaListos,i);
			pthread_mutex_unlock(&mutex_Listos);
			int cantbytes = getpeername(jugador->socket, &mysock,&addrlen);

			if(cantbytes){
				jugador = list_remove(colaListos,i);
				desconectarJugador(jugador);

			}

			if(errno == ENOTCONN)
			{
				jugador = list_remove(colaListos,i);
				desconectarJugador(jugador);
			}


		}

		//free(addrlen);
}


void detectarDesconexiones5()
{

}

int verificarConexion(Jugador* jugador,int retval,int* quantum)
{
	if(retval < 0)
	{
		desconectarJugador(jugador);
		*quantum = 0;
		return 1;
	}

	return 0;
}

#endif /* HEADERS_DESCONEXIONES_H_ */
