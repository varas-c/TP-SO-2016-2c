/*
 * serializeEntrenador.h
 *
 *  Created on: 13/9/2016
 *      Author: utnso
 */

#ifndef HEADERS_SERIALIZEENTRENADOR_H_
#define HEADERS_SERIALIZEENTRENADOR_H_

enum codigoOperaciones {
	TURNO = 0,
	POKENEST = 1,
	MOVER = 2,
	CAPTURAR = 3,
	FINOBJETIVOS = 4,
	SIMBOLO = 10,
	COORDENADAS = 11,
	CAPTURA_OK = 12,
	MOVER_OK = 13,
	CAPTURA_BLOQUEADO = 14,
	BATALLA_PERDIDA = 15,
	BATALLA_PELEA = 16,
	PEDIR_POKEMON_MAS_FUERTE=18,
	BATALLA_INFORME =19,
	BATALLA_MUERTE=20,
	BATALLA_GANADOR = 21,
};

//****************************************************************************************************************

enum sizeofBuffer
{
	size_TURNO = sizeof(int),
	size_POKENEST_response = sizeof(int) + sizeof(char)+sizeof(int)+sizeof(int),
	size_POKENEST_request = sizeof(int) + sizeof(char),
	size_MOVER = sizeof(int)+sizeof(int)+sizeof(int),
	size_CAPTURAR = sizeof(int) + sizeof(char),
	size_SIMBOLO = sizeof(int)+sizeof(char),
	size_FINOBJETIVOS = sizeof(int),
	size_COORDENADAS = sizeof(int)+sizeof(int)+sizeof(int),
	size_CAPTURA_OK = sizeof(int)+sizeof(int),
	size_MOVER_OK = sizeof(int),
	size_MENSAJECAPTURA = sizeof(int),
	size_PEDIR_POKEMON_MAS_FUERTE= sizeof(int)+sizeof(int),
	size_BATALLA_INFORME =sizeof(int)+sizeof(int),
};
//******************************************

Paquete srlz_movEntrenador(Entrenador entrenador)
{
	Paquete paquete;
	paquete.buffer = malloc(size_MOVER);
	paquete.tam_buffer = size_MOVER;

	int codOp = MOVER;

	memcpy(paquete.buffer,&codOp,sizeof(int));
	memcpy(paquete.buffer+sizeof(int),&(entrenador.posx),sizeof(int));
	memcpy(paquete.buffer+sizeof(int)*2,&(entrenador.posy),sizeof(int));
	return paquete;
}
//******************************************

Paquete srlz_simboloEntrenador(char simbolo)
{
	Paquete paquete;
	paquete.buffer = malloc(size_SIMBOLO);
	paquete.tam_buffer = size_SIMBOLO;
	int codigo = SIMBOLO;

	memcpy(paquete.buffer,&codigo,sizeof(int));
	memcpy(paquete.buffer+sizeof(int),&simbolo,sizeof(char));
	return paquete;
}
//******************************************

Paquete srlz_capturarPokemon(char simbolo)
{
	Paquete paquete;
	paquete.buffer = malloc(size_CAPTURAR);
	paquete.tam_buffer = size_CAPTURAR;

	int codigo = CAPTURAR;

	memcpy(paquete.buffer,&codigo,sizeof(int));
	memcpy(paquete.buffer+sizeof(int),&simbolo,sizeof(char));
	return paquete;
}
//******************************************

Paquete srlz_pokemonMasFuerte(int indicePok)
{
	Paquete paquete;
	paquete.buffer=malloc(size_PEDIR_POKEMON_MAS_FUERTE);
	paquete.tam_buffer= size_PEDIR_POKEMON_MAS_FUERTE;

	int codigo = PEDIR_POKEMON_MAS_FUERTE;

	memcpy(paquete.buffer,&codigo,sizeof(int));
	memcpy(paquete.buffer+sizeof(int),&indicePok,sizeof(int));
	return paquete;
}
//******************************************

Paquete srlz_finObjetivos()
{
	Paquete paquete;
	paquete.buffer = malloc(size_FINOBJETIVOS);
	paquete.tam_buffer = size_FINOBJETIVOS;

	int codigo = FINOBJETIVOS;

	memcpy(paquete.buffer,&codigo,sizeof(int));

	return paquete;
}
//******************************************

Paquete srlz_solicitarPokenest(Pokenest pokenest)
{
	Paquete paquete;
	paquete.buffer = malloc(size_POKENEST_request);
	paquete.tam_buffer = size_POKENEST_request;

	int codigo = POKENEST;

	//Copiamos primero el codigo, despues el simbolo de la Pokenest
	memcpy(paquete.buffer,&codigo,sizeof(int));
	memcpy(paquete.buffer+sizeof(int),&(pokenest.simbolo),sizeof(char));

	return paquete;
}

//************************************************************************

int dsrlz_codigoOperacion(void* buffer)
{
	int codOp;
	memcpy(&codOp,buffer,sizeof(int));
	return codOp;
}
//******************************************

char* dsrlz_capturarPokemon(Paquete* paquete, Entrenador* entrenador)
{
	int codOp;
	int lengthPokemonDat;
	char *pokemonDat;
	char* species;
	int tamSpecies;
	int* pokemonLevel;

	codOp = dsrlz_codigoOperacion(paquete->buffer);

	if(codOp != CAPTURA_OK)
	{
		perror("dsrlz_capturarPokemon - Codigo de Operacion CAPTURA_OK invalido");
		exit(1);
	}

	memcpy(&lengthPokemonDat,paquete->buffer+sizeof(int),sizeof(int));

	pokemonDat = malloc(sizeof(char)*lengthPokemonDat+1);

	memcpy(pokemonDat,paquete->buffer+sizeof(int)*2,sizeof(char)*lengthPokemonDat);

	pokemonLevel = malloc(sizeof(int));

	memcpy(pokemonLevel,paquete->buffer+sizeof(int)*2+sizeof(char)*lengthPokemonDat,sizeof(int));


	list_add(entrenador->pokemonesCapturados,pokemonLevel);

	return pokemonDat;
}
//******************************************

int dsrlz_MoverOK(void* buffer)
{
	int codOp = 0;
	memcpy(&codOp,buffer,sizeof(int));
	return codOp;
}

void dsrlz_BatallaInforme(Paquete* paquete, int fdServer)
{
	int codOp =0;
	int tamInforme=0;
	char* informeBatalla;

	codOp = dsrlz_codigoOperacion(paquete->buffer);

	if(codOp != BATALLA_INFORME)
	{
		printf("\n\nFuncion: %s - Linea:%d - Error:codigo de Operacion BATALLA_INFORME invalido - codOp %i\n\n",__func__,__LINE__,codOp);
		exit(1);
	}

	memcpy(&tamInforme,paquete->buffer+sizeof(int),sizeof(int));

	free(paquete->buffer);

	paquete->tam_buffer = tamInforme;
	paquete->buffer = malloc(tamInforme);

	recv(fdServer,paquete->buffer,tamInforme,0);

	informeBatalla = malloc(sizeof(char)*tamInforme+1);

	memcpy(informeBatalla,paquete->buffer,sizeof(char)*tamInforme);

	printf("\n\n %s \n\n ", informeBatalla);

	free(informeBatalla);

}

#endif /* HEADERS_SERIALIZEENTRENADOR_H_ */
