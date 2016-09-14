/*
 * send.h
 *
 *  Created on: 13/9/2016
 *      Author: utnso
 */

#ifndef HEADERS_SEND_H_
#define HEADERS_SEND_H_
#include "serializeEntrenador.h"

void send_solicitarPokenest(Pokenest *pokenest, int fd_server)
{
	void* buffer;
	int tam_buffer = sizeof(int)+sizeof(char);
	buffer = malloc(tam_buffer);

	int codigo = POKENEST;

	//Copiamos primero el codigo, despues el simbolo de la Pokenest
	memcpy(buffer,&codigo,sizeof(int));
	memcpy(buffer+sizeof(int),&(pokenest->simbolo),sizeof(char));

	send(fd_server, buffer, tam_buffer,0);

	free(buffer);
}
//****************************************//****************************************

void send_capturarPokemon(Paquete *paquete,int server)
{
	send(server,paquete->buffer,paquete,0);
}
//****************************************//****************************************

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
