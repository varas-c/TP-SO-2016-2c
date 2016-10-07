/*
 * pokenest.h
 *
 *  Created on: 6/10/2016
 *      Author: utnso
 */

#ifndef HEADERS_POKENEST_H_
#define HEADERS_POKENEST_H_

int faltaPokenest(Pokenest pokenest)//Si la pokenest tiene posx y posy = -1 es porque no tenemos datos,hay que leer
{
	return pokenest.posx == -1 || pokenest.posy == -1;
}
//****************************************//****************************************

int llegueAPokenest(const Entrenador entrenador, const Pokenest pokenest)
{
	if(entrenador.posx == pokenest.posx && entrenador.posy == pokenest.posy)
	{
		return 1;
	}
	return 0;
}
//****************************************//****************************************

void recv_solicitarPokenest(Pokenest* pokenest, int fd_server)
{
	int valor_recv;
	int codigo;
	int tam_buffer = size_POKENEST_response;

	char* buffer = malloc(tam_buffer);

	valor_recv = recv(fd_server, buffer, tam_buffer, 0);// Se almacena el mensaje recibido en BUFFER

	if(valor_recv == -1)  {
		perror("Error recv"); //Hubo fallo, chau programa.
		exit(1);
	}

	//Verificamos que el codigo sea correcto
	memcpy(&codigo,buffer,sizeof(int));

	if(codigo != POKENEST)
	{
		perror("CODIGO DE ERROR - RECV POKENEST");
		exit(1);
	}

	char simboloAux;

	memcpy(&simboloAux,buffer+sizeof(int),sizeof(char));

	if(pokenest->simbolo != simboloAux)
	{
		exit(1);
	}

	memcpy(&(pokenest->posx),buffer+sizeof(int)+sizeof(char),sizeof(int));
	memcpy(&(pokenest->posy),buffer+sizeof(int)+sizeof(char)+sizeof(int),sizeof(int));

	free(buffer);
}

//********************************
void calcular_coordenadas(Entrenador* entrenador, int x, int y)
{
	//Pregunto por X

	if(x > entrenador->posx)
	{
		entrenador ->destinox = x - entrenador->posx;
	}

	if(entrenador->posx == x)
	{
		entrenador ->destinox = 0;
	}

	if(x < entrenador->posx)
	{
		entrenador->destinox = x - entrenador->posx;
	}

	//Pregunto por y

	if(y > entrenador->posy)
	{
		entrenador ->destinoy = y - entrenador->posy;
	}

	if(entrenador->posy == y)
	{
		entrenador ->destinoy = 0;
	}

	if(y < entrenador->posy)
	{
		entrenador->destinoy = y - entrenador->posy;
	}
}

//*********************************
//********************************

void send_coordenadas(Entrenador entrenador)
{

}




#endif /* HEADERS_POKENEST_H_ */
