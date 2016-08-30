/*
 * struct.h
 *
 *  Created on: 30/8/2016
 *      Author: utnso
 */

#ifndef HEADERS_STRUCT_H_
#define HEADERS_STRUCT_H_

typedef struct
{
	char* nombre;
	char simbolo;
	char** hojaDeViaje;
	char** objetivos;
	int vidas;
	int reintentos;

}metadata;


typedef struct
{
	char* nombreEntrenador;
	char* dirPokedex;
}ParametrosConsola;


typedef struct
{
	char* ip;
	char* puerto;
}ConexionEntrenador;

#endif /* HEADERS_STRUCT_H_ */
