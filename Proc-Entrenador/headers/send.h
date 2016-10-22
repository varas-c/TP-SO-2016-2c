/*
 * send.h
 *
 *  Created on: 13/9/2016
 *      Author: utnso
 */

#ifndef HEADERS_SEND_H_
#define HEADERS_SEND_H_
#include "serializeEntrenador.h"

void send_solicitarPokenest(Paquete* paquete, int fd_server)
{
	send(fd_server, paquete->buffer, paquete->tam_buffer,0);
}
//****************************************//****************************************

void send_capturarPokemon(Paquete *paquete,int server)
{
	send(server,paquete->buffer,paquete->tam_buffer,0);
}
//****************************************//****************************************

void send_pokemonMasFuerte(Paquete *paquete, int socket)
{
	send(socket, paquete->buffer,paquete->tam_buffer,0);
		free(paquete->buffer);
}
//*********************************************************************************

void send_movEntrenador(Paquete *paquete, int socket)
{
	send(socket,paquete->buffer,paquete->tam_buffer,0);
}
//****************************************//****************************************

void send_simboloEntrenador(char simbolo,int socket)
{
	Paquete paquete;
	paquete = srlz_simboloEntrenador(simbolo);
	send(socket,paquete.buffer,paquete.tam_buffer,0);
	free(paquete.buffer);
}
//****************************************//****************************************

void send_finObjetivos(Paquete* paquete, int socket)
{
	send(socket,paquete->buffer,paquete->tam_buffer,0);
}

#endif /* HEADERS_SEND_H_ */
