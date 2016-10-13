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
