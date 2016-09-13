/*
 * serialize.h
 *
 *  Created on: 13/9/2016
 *      Author: utnso
 */

#ifndef HEADERS_SERIALIZEMAPA_H_
#define HEADERS_SERIALIZEMAPA_H_

/****************************************************************************************************************
 * ************************************************************************************************************
 *
			ENUMS PARA LAS OPERACIONES DE SRLZ, DSRLZ, SEND Y RECV

****************************************************************************************************************
****************************************************************************************************************/
enum codigoOperaciones {
	TURNO = 0,
	POKENEST = 1,
	MOVER = 2,
	SIMBOLO = 10
};

enum sizeofBuffer
{
	size_TURNO = sizeof(int),
	size_POKENEST = sizeof(int) + sizeof(char)+sizeof(int)+sizeof(int),
	size_MOVER = sizeof(int)+sizeof(int)+sizeof(int),
	size_SIMBOLO = sizeof(int)+sizeof(char)
};

//****************************************************************************************************************
/****************************************************************************************************************
 * ************************************************************************************************************
 *
			SERIALIZADO Y DESERIALIZADO

****************************************************************************************************************
****************************************************************************************************************/
//****************************************************************************************************************

char dsrlz_Pokenest(void* buffer)
{
	char pokenest;
	memcpy(&pokenest,buffer+sizeof(int),sizeof(char));
	return pokenest;
}

//****************************************************************************************************************
//****************************************************************************************************************


Paquete srlz_Pokenest(MetadataPokenest pokenest)
{
	Paquete paquete;
	paquete.buffer = malloc(size_POKENEST);
	paquete.tam_buffer = size_POKENEST;

	int size[4];
	size[0] = sizeof(int);
	size[1] = sizeof(char);
	size[2] = sizeof(int);
	size[3] = sizeof(int);

	//Copiamos el codigo de operacion
	int codigo = POKENEST;
	memcpy(paquete.buffer,&codigo,size[0]);

	//Copiamos el simbolo
	memcpy(paquete.buffer + size[0], &(pokenest.simbolo),size[1]);

	//Copiamos la coordenada en X
	memcpy(paquete.buffer+size[0]+size[1],&(pokenest.posicionX),size[2]);

	//Copiamos la coordenada en Y
	memcpy(paquete.buffer+size[0]+size[1]+size[2],&(pokenest.posicionY),size[3]);

	return paquete;
}

//****************************************************************************************************************
//****************************************************************************************************************

Paquete srlz_turno()
{
	Paquete paquete;
	paquete.buffer = malloc(size_TURNO);
	paquete.tam_buffer = size_TURNO;
	int turno = TURNO;

	memcpy(paquete.buffer,&turno,size_TURNO);

	return paquete;
}

//****************************************************************************************************************
//****************************************************************************************************************

PosEntrenador dsrlz_movEntrenador(void* buffer)
{
	PosEntrenador pos;

	memcpy(&(pos.x),buffer+sizeof(int),sizeof(int));
	memcpy(&(pos.y),buffer+sizeof(int)+sizeof(int),sizeof(int));

	return pos;
}

//****************************************************************************************************************
//****************************************************************************************************************

char dsrlz_simboloEntrenador(void* buffer)
{
	int codigo;
	char simbolo;

	memcpy(&codigo,buffer,sizeof(int));

	if(codigo != SIMBOLO)
	{
		perror("Error Codigo de Operacion: Simbolo dsrlz_simboloEntrenador() ");
		exit(1);
	}

	memcpy(&simbolo,buffer+sizeof(int),sizeof(char));

	return simbolo;

}


//****************************************************************************************************************
//****************************************************************************************************************

int dsrlz_codigoOperacion(void* buffer)
{
	int codOp;
	memcpy(&codOp,buffer,sizeof(int));
	return codOp;
}

#endif /* HEADERS_SERIALIZEMAPA_H_ */
