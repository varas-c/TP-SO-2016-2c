#include <stdio.h>
#include <tad_items.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <curses.h>
#include <math.h>
#include <ctype.h>
#include <errno.h>

void copiarPokemon(char *archivoPokemon, char* montaje, char* nombreEntrenador, char* nombreMapa){
	char *origen, *destino, *nombre, *comandoCopiar;
	int tamanio;
	tamanio = strlen(archivoPokemon);
	nombre = malloc(tamanio - 6);
	memcpy(nombre,archivoPokemon,tamanio-7);

	char* barraCero = "\0";
	memcpy(nombre+tamanio-7,barraCero,sizeof(char));
	origen = malloc(sizeof(char)*256);
	destino = malloc(sizeof(char)*256);

	strcpy(origen, montaje);
	strcat(origen, "/Mapas/");
	strcat(origen, nombreMapa);
	strcat(origen, "/Pokenest/");
	strcat(origen, nombre);
	strcat(origen, "/");
	strcat(origen, archivoPokemon);

	strcpy(destino, montaje);
	strcat(destino, "/Entrenadores/");
	strcat(destino, nombreEntrenador);
	strcat(destino, "/DirdeBill");

	comandoCopiar = malloc(sizeof(origen)+sizeof(destino)+5);
	sprintf(comandoCopiar, "cp %s %s", origen, destino);

	system(comandoCopiar);

	origen = NULL;
	destino = NULL;
	nombre = NULL;
	comandoCopiar = NULL;

	free(origen);
	free(nombre);
	free(destino);
	free(comandoCopiar);
}

void borrarPokemones(char* entrenador){
	char *directorio, *comandoBorrar;
	directorio = malloc(sizeof(char)*256);

	strcpy(directorio, "/mnt/pokedex");
	strcat(directorio, "/Entrenadores/");
	strcat(directorio, entrenador);
	strcat(directorio, "/DirdeBill/");

	comandoBorrar = malloc(sizeof(directorio)+9);
	sprintf(comandoBorrar, "rm -rf %s*", directorio);

	system(comandoBorrar);

	directorio = NULL;
	comandoBorrar = NULL;

	free(directorio);
	free(comandoBorrar);
}

int main(){
	//copiarPokemon("Bulbasaur001.dat", "/mnt/pokedex", "Chris", "PuebloPaleta");
	borrarPokemones("Chris");
	getchar();
	return 0;
}
