/*
 * batalla.c
 *
 *  Created on: 19/9/2016
 *      Author: utnso
 */
#include <stdio.h>
#include <string.h>
#include "struct.h"
#include <stdbool.h>
#include "nivel.h"
#include <commons/collections/list.h>
#include <commons/config.h>
#include "configPokemon.h"
#include <pkmn/battle.h>
#include <pkmn/factory.h>
#include <curses.h>
#include <stdlib.h>


t_pokemon generarPokemon(MetadataPokemon* mdata,t_pkmn_factory* fabrica, char* especie)
{
	return *create_pokemon(fabrica, especie, mdata->nivel);
}

int main (void)
{
	t_pokemon poke1, poke2;
	MetadataPokemon mdata1,mdata2;
	char* buffer=malloc(25);
	strcpy(buffer,"Pikachu001.dat");

	mdata1 = leerMetadataPokemon(buffer);

	strcpy(buffer,"Squirtle001.dat");

	mdata2 = leerMetadataPokemon(buffer);

	t_pkmn_factory* fabrica = create_pkmn_factory();     //SE CREA LA FABRICA PARA HACER POKEMONES

	poke1 = generarPokemon(&mdata1,fabrica,"Pikachu");
	poke2 = generarPokemon(&mdata2,fabrica,"Squirtle");

	t_pokemon* perdedor = pkmn_battle(&poke1, &poke2);

	printf("El perdedor es: %s",perdedor->species);

}
