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
	int puerto;
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
	t_queue* colaDePokemon;
	int cantPokemon;
}MetadataPokenest;
//****************************************************************************************************************

typedef struct   //MetadataPokemon
{	int nivel;
}MetadataPokemon;
//****************************************************************************************************************

typedef struct{  //Entrenador
	char simbolo;
	int posx;
	int posy;
	int destinox;
	int destinoy;
	char* pokemons;
}Entrenador;

//****************************************************************************************************************

typedef struct   //Jugador
{	Entrenador entrenador;
	t_list* pokemonCapturados;
	int estado;
	int socket;
	int numero;
}Jugador;
//****************************************************************************************************************

typedef struct   //Pokenest
{
	int posx;
	int posy;
	char simbolo;
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

//****************************************************************************************************************

typedef struct
{
	t_pokemon* pokemon;
	char* nombre;
	int numero;
}Pokemon;


//***************//********!!!!!!!!!!!NEW DE STRUCTS!!!!!!!!!!!!!!//***************//***************


Entrenador new_Entrenador(char simbolo)
{
	Entrenador entrenador;

    entrenador.posx = 1;
    entrenador.posy = 1;
    entrenador.simbolo = simbolo;
    entrenador.pokemons = NULL;
    entrenador.destinox = -1;
    entrenador.destinoy = -1;

    return entrenador;
}
//****************************************************************************************************************

Jugador new_Jugador(char simbolo, int socket, int numero){

	Jugador jugador;

	jugador.entrenador = new_Entrenador(simbolo);
	jugador.socket = socket;
	jugador.estado = 0;
	jugador.numero = numero;
	jugador.pokemonCapturados = list_create();

	return jugador;
}

#endif /* HEADERS_STRUCTS_H_ */
