/*
 * structs.h
 *
 *  Created on: 30/8/2016
 *      Author: utnso
 */

#ifndef HEADERS_STRUCTS_H_
#define HEADERS_STRUCTS_H_

/* -----------------------------------------------
 * METADATA DEL MAPA
 * Almacena la información que se lee desde el archivo Metadata del Mapa
 ------------------------------------------------- */

typedef struct
{
	int tiempoChequeoDeadlock;
	int modoBatalla;
	char* algoritmo;
	int quantum;
	int retardo;
	char* ip;
	char* puerto;
}MetadataMapa;

/* -----------------------------------------------
 * PARAMETROS DEL MAPA
 * Almacena la información que se lee desde la consola al ejecutar el mapa
 * Los parametros se leen desde **argv
 ------------------------------------------------- */

typedef struct
{
	char* nombreMapa;
	char* dirPokedex;

}ParametrosMapa;

/* -----------------------------------------------
 * METADATAPOKENEST
 * Almacena la información de las PokeNest
 ------------------------------------------------- */

typedef struct
{
	char* tipoPokemon;
	int posicionX;
	int posicionY;
	char simbolo;
}MetadataPokenest;

/* -----------------------------------------------
 * METADATAPOKEMON
 * Almacena la información de los Pokemon
 ------------------------------------------------- */

typedef struct
{
	int nivel;
}MetadataPokemon;

/* -----------------------------------------------
 * Poekenst
 * Esctructura para crear Pokenest
 ------------------------------------------------- */

typedef struct{
	char simbolo;
	int posx;
	int posy;
	char movAnterior;
	int flagx;
	int flagy;
	char* pokemones;
}Entrenador;

//***************//***************
//***************//***************
typedef struct
{
	Entrenador entrenador;
	int estado;
	int socket;
}Jugador;


//***************//***************
//***************//***************
typedef struct
{
	int posx;
	int posy;
	char simbolo;
}Pokenest;

//***************
//***************



#endif /* HEADERS_STRUCTS_H_ */
