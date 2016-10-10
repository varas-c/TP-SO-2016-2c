/*
 * entrenador.h
 *
 *  Created on: 13/9/2016
 *      Author: utnso
 */

#ifndef HEADERS_ENTRENADOR_H_
#define HEADERS_ENTRENADOR_H_

void movEntrenador(PosEntrenador pos, Jugador* jugador)
{
	jugador->entrenador.posx = pos.x;
	jugador->entrenador.posy = pos.y;
}
//****************************************************************************************************************

char recv_simboloEntrenador(int socket)
{
	char simbolo;

	Paquete paquete;
	paquete.buffer = malloc(size_SIMBOLO);
	paquete.tam_buffer = size_SIMBOLO;

	recv(socket,paquete.buffer,paquete.tam_buffer,0);

	simbolo = dsrlz_simboloEntrenador(paquete.buffer);

	return simbolo;
}

#endif /* HEADERS_ENTRENADOR_H_ */
