/*
 * struct.h
 *
 *  Created on: 30/8/2016
 *      Author: utnso
 */



#ifndef HEADERS_STRUCT_H_
#define HEADERS_STRUCT_H_


/* -----------------------------------------------
 * METADATA DEL ENTRENADOR
 * Almacena la información que se lee desde el archivo Metadata del Entrenador
 ------------------------------------------------- */

typedef struct
{
	char* nombre;
	char simbolo;
	char** hojaDeViaje;
	char** objetivos;
	int vidas;
	int reintentos;
}metadata;

/* -----------------------------------------------
 * ParametrosConsola
 * Almacena la información que se lee desde la consola
 ------------------------------------------------- */

typedef struct
{
	char* nombreEntrenador;
	char* dirPokedex;
}ParametrosConsola;

/* -----------------------------------------------
 * ConexionEntrenador
 * Almacena la información que se lee desde el archivo del Mapa donde esta la IP y Puerto
 ------------------------------------------------- */

typedef struct
{
	char* ip;
	char* puerto;
}ConexionEntrenador;

/* -----------------------------------------------
 * Entrenador
 * Estructura para crear un jugador y poder moverlo, mostrarlo por pantalla.
 ------------------------------------------------- */

typedef struct{
	char simbolo;
	int posx;
	int posy;
	char movAnterior;
	int flagx;
	int flagy;
	int destinox;
	int destinoy;
	int vidas;
	int reintentos;
	char* nombre;
}Entrenador;


typedef struct
{
	int posx;
	int posy;
	char simbolo;
}Pokenest;

typedef struct
{
	void* buffer;
	int tam_buffer;
}Paquete;


#endif /* HEADERS_STRUCT_H_ */
