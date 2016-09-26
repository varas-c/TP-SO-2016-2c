/*
 * structs.h
 *
 *  Created on: 30/8/2016
 *      Author: utnso
 */

#ifndef HEADERS_STRUCTS_H_
#define HEADERS_STRUCTS_H_

typedef struct    //MetadataMapa
{
	int tiempoChequeoDeadlock;
	int modoBatalla;
	char* algoritmo;
	int quantum;
	int retardo;
	char* ip;
	char* puerto;
}MetadataMapa;
//****************************************************************************************************************

typedef struct   //ParametrosMapa,Almacena la información que se lee desde la consola al ejecutar el mapa, los parametros se leen desde **argv
{
	char* nombreMapa;
	char* dirPokedex;

}ParametrosMapa;
//****************************************************************************************************************

typedef struct   //MetadataPokenest
{	char* tipoPokemon;
	int posicionX;
	int posicionY;
	char simbolo;
}MetadataPokenest;
//****************************************************************************************************************

typedef struct   //MetadataPokemon
{	int nivel;
}MetadataPokemon;
//****************************************************************************************************************

typedef struct{
	char simbolo;
	int posx;
	int posy;
	char movAnterior;
	int flagx;
	int flagy;
	int destinox;
	int destinoy;
	char* pokemones;
}Entrenador;
//****************************************************************************************************************

typedef struct   //Jugador
{	Entrenador entrenador;
	int estado;
	int socket;
}Jugador;
//****************************************************************************************************************

typedef struct   //Pokenest
{
	int posx;
	int posy;
	char simbolo;
	int cant;
}Pokenest;
//****************************************************************************************************************

typedef struct   //Paquete
{
	void* buffer;
	int tam_buffer;
}Paquete;
//****************************************************************************************************************

typedef struct   //PosEntrenador
{
	int x;
	int y;
}PosEntrenador;


//***************//********!!!!!!!!!!!NEW DE STRUCTS!!!!!!!!!!!!!!//***************//***************


Entrenador new_Entrenador(char simbolo)
{
	Entrenador entrenador;

    entrenador.posx = 1;
    entrenador.posy = 1;
    entrenador.simbolo = simbolo;
    entrenador.movAnterior = 'y';
    entrenador.flagx = 0;
    entrenador.flagy = 0;
    entrenador.pokemones = NULL;

    return entrenador;
}
//****************************************************************************************************************

Jugador new_Jugador(char simbolo, int socket){

	Jugador jugador;

	jugador.entrenador = new_Entrenador(simbolo);
	jugador.socket = socket;
	jugador.estado = 0;

	return jugador;
}

#endif /* HEADERS_STRUCTS_H_ */
