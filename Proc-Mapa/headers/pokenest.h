/*
 * pokenest.h
 *
 *  Created on: 13/9/2016
 *      Author: utnso
 */

#ifndef HEADERS_POKENEST_H_
#define HEADERS_POKENEST_H_

t_list* listaPokenest;

MetadataPokenest buscar_Pokenest(char simbolo)
{
	bool _find_pokenest_(MetadataPokenest* aux)
	{
		return aux->simbolo == simbolo;
	}

	MetadataPokenest *ptr = (MetadataPokenest*) list_find(listaPokenest,(void*)_find_pokenest_);

	MetadataPokenest pokenest;
	pokenest.simbolo = ptr->simbolo;
	pokenest.posicionX = ptr->posicionX;
	pokenest.posicionY = ptr->posicionY;
	pokenest.tipoPokemon = strdup(ptr->tipoPokemon);

	return pokenest;
}
//****************************************************************************************************************

void send_Pokenest(int socket,Paquete *paquete)
{
	send(socket,paquete->buffer,paquete->tam_buffer,0);
}

#endif /* HEADERS_POKENEST_H_ */
