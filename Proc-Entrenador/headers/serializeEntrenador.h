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
	SIMBOLO = 10
};


enum sizeofBuffer
{
	size_TURNO = sizeof(int),
	size_POKENEST = sizeof(int) + sizeof(char)+sizeof(int)+sizeof(int),
	size_MOVER = sizeof(int)+sizeof(int)+sizeof(int),
	size_CAPTURAR = sizeof(int) + sizeof(char),
	size_SIMBOLO = sizeof(int)+sizeof(char),
	size_FINOBJETIVOS = sizeof(int)
};

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

Paquete srlz_finObjetivos()
{
	Paquete paquete;
	paquete.buffer = malloc(size_FINOBJETIVOS);
	paquete.tam_buffer = size_FINOBJETIVOS;

	int codigo = FINOBJETIVOS;

	memcpy(paquete.buffer,&codigo,sizeof(int));

	return paquete;
}


#endif /* HEADERS_SERIALIZEENTRENADOR_H_ */
