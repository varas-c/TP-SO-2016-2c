/*
 * deadlock.c
 *
 *  Created on: 19/9/2016
 *      Author: utnso
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <curses.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/config.h>
#include <tad_items.h>
#include <pkmn/battle.h>
#include <pkmn/factory.h>
#include "headers/struct.h"
#include <ctype.h>

/*

typedef struct{
	char simbolo;
	int nivel;
}PokemonDL;

typedef struct{
	t_list* recursos;
	char peticion;
}EntrenadorDL;

typedef struct{
	char simbolo;
	char *species;
	int cant;
}PokenestDL;

*/

int** inicializar_matriz(int cant_filas, int cant_columnas)
{
	int i=0,j=0;
	int** matriz;
	matriz=malloc(cant_filas*sizeof(int));
	for(j=0;j<cant_filas;j++)
	{
		matriz[j]=malloc(cant_columnas*sizeof(int));
	}
	for(i=0;i<cant_filas;i++)
	{
		for(j=0;j<cant_columnas;j++)
		{
			matriz[i][j]=0;
		}
	}
	return matriz;
}

void mostrar_matriz(int** matriz,int filas, int columnas)
{
	int i,j;
	for(i=0;i<filas;i++)
	{
		printf("ENTRENADOR %d     ",i+1);
		for(j=0;j<columnas;j++)
		{
			printf(" %d ",matriz[i][j]);
		}
		printf("\n");
	}
	printf("\n");
	printf("\n");
}

void mostrar_recursos_disponibles(int* recursos, int cant_recursos)
{
	int i=0;
	printf("RECURSOS          ");
	for (i=0;i<cant_recursos;i++)
	{
		printf("%d  ", recursos[i]);
	}
}


int cantidad_obtenidos_de_un_tipo(Jugador* entrenador,char simbolo)//CANT DE POKEMONES ASIGNADOS A ESE ENTRENADOR DE UNA POKENEST
{
	int i=0,cantidad_total,cantidad=0;
	Pokemon* pokemon_aux;
	cantidad_total = list_size(entrenador->pokemonCapturados);
	while(i<cantidad_total)
	{
		pokemon_aux=(Pokemon*)list_get(entrenador->pokemonCapturados,i);
		if(letra_pokenest(pokemon_aux->nombre)==simbolo) //SI ESE POKEMON ES DE ESA POKENEST
		{
			cantidad++;
		}
		i++;
	}
	return cantidad;
}

int** generar_matriz_peticiones(t_list* entrenadores, t_list* pokenests)
{
	Jugador* entrenador_aux;
	MetadataPokenest* pokenest_aux;
	int i,j,cant_pokenests,cant_entrenadores,cantidad_recursos;

	cant_pokenests=list_size(pokenests);
	cant_entrenadores=list_size(entrenadores);

	int** matriz= inicializar_matriz(cant_entrenadores, cant_pokenests);//DEVUELVE MATRIZ CON TODOS 0


	i = 0;

	while(i<cant_pokenests)
	{
		pokenest_aux = (MetadataPokenest*)list_get(pokenests,i);
		j = 0;
		while(j<cant_entrenadores)
		{
			entrenador_aux = (Jugador*)list_get(entrenadores,j);
			if(entrenador_aux->peticion == pokenest_aux->simbolo)
			{
				matriz[j][i]=1;
			}

				j++;
		}
		i++;
	}
	return matriz;
}

int** generar_matriz_asignados(t_list* entrenadores, t_list* pokenests)
{
	Jugador* entrenador_aux;
	MetadataPokenest* pokenest_aux;
	int i=0,j=0,cant_pokenests,cant_entrenadores,cantidad_recursos;

	cant_pokenests=list_size(pokenests);

	cant_entrenadores=list_size(entrenadores);


	int** matriz= inicializar_matriz(cant_entrenadores, cant_pokenests);//DEVUELVE MATRIZ CON TODOS 0

	while(i<cant_pokenests)
	{
		cantidad_recursos = 0;
		pokenest_aux = (MetadataPokenest*)list_get(pokenests,i);
		j=0;
		while(j<cant_entrenadores)
		{
			entrenador_aux = (Jugador*)list_get(entrenadores,j);
			cantidad_recursos = cantidad_obtenidos_de_un_tipo(entrenador_aux, pokenest_aux->simbolo); //CANT DE POKEMONES ASIGNADOS A ESE ENTRENADOR DE ESA POKENEST
			matriz[j][i]=cantidad_recursos;
			j++;
		}
		i++;
	}
	return matriz;
}

int* generar_vector_recursos_disponibles(t_list* pokenests)
{
	int i=0,tamanio;
	int* vector=malloc(sizeof(int)*(list_size(pokenests)));
	tamanio=list_size(pokenests);
	while(i<tamanio)
	{
		vector[i]=((MetadataPokenest*)list_get(pokenests,i))->cantPokemon;
		i++;
	}
	return vector;
}

t_list * no_pueden_ejecutar(t_list* entrenadores, t_list*pokenests)
{
	int **matriz_peticiones,**matriz_recursos_asignados,*recursos_disponibles,cant_entrenadores, cant_pokenests;
	bool tiene_algun_recurso,pide_algun_recurso;
	cant_entrenadores = list_size(entrenadores);
	cant_pokenests = list_size(pokenests);
	int posible_deadlock[cant_entrenadores];
	int i,j;

	matriz_peticiones = generar_matriz_peticiones(entrenadores, pokenests);

	matriz_recursos_asignados = generar_matriz_asignados(entrenadores,pokenests);

	recursos_disponibles = generar_vector_recursos_disponibles(pokenests);

	t_list* no_puede_ejecutar = list_create();
    for(i=0;i<cant_entrenadores;i++)
    {
    	posible_deadlock[i]=1;  // CULPABLE HASTA QUE SE DEMUESTRE LO CONTRARIO
    }


	for(i=0;i<cant_entrenadores;i++)  //RECORRO CADA ENTRENADOR
	{	tiene_algun_recurso = false;
		pide_algun_recurso=false;
		for(j=0;j<cant_pokenests;j++)  //RECORRO CADA POKENEST (RECURSO)
		{
			if(matriz_recursos_asignados[i][j] > 0)
			{
				tiene_algun_recurso = true;
			}
		    if(matriz_peticiones[i][j]>0)
		    {
		    	pide_algun_recurso = true;
		    	if(matriz_peticiones[i][j] <= recursos_disponibles[j])
		    	{
					posible_deadlock[i] = 0;
					j=cant_pokenests;   //    ESTE ENTRENADOR PUEDE OBTENER EL POKEMON DE LA POKENEST
		    	}
		    }
		}
		if((!tiene_algun_recurso)||(!pide_algun_recurso))
		{
			posible_deadlock[i] = 0;    //NO TIENE RECURSO ASIGNADO, NO CUMPLE "ESPERA"
		}
	}

	j=0;

	for(i=0;i<cant_entrenadores;i++)
	{
		if(posible_deadlock[i])
		{
			list_add(no_puede_ejecutar,list_get(entrenadores,i)); // DEVUELVO LISTA CON LOS QUE PUEDEN ESTAR EN DL
			j++;
		}
	}


	return no_puede_ejecutar; //DEVUELVO LOS QUE NO PUEDEN EJECUTAR, FALTA EVALUAR LA ESPERA CIRCULAR
}


bool tiene_lo_que_pide(Jugador* entrenador, Jugador* otro_entrenador)
{
	int i;
	Pokemon* poke1;
	for(i=0;i<list_size(otro_entrenador->pokemonCapturados);i++)
	{
		poke1=(Pokemon*)list_get(entrenador->pokemonCapturados,i);

		if(((char)letra_pokenest(poke1->nombre))==entrenador->peticion)
		{
			return true;
		}
	}

	return false;
}


////BUSCAMOS ESPERA CIRCULAR

t_list* obtener_un_deadlock(t_list* entrenadores)    //ME DEVUELVE UN DEADLOCK
{
	t_list* un_deadlock = list_create();

	Entrenador *aux,*primero, *pivote;

	int i=0;

	primero = ((Jugador*)list_get(entrenadores,0));//COMO TODOS ESTAN EN DEADLOCK, AGARRO UNO Y VEO CON CUALES ESTA EN DEADLOCK

	pivote = ((Jugador*)list_get(entrenadores,0));
	list_add(un_deadlock,primero);
	list_remove(entrenadores,0);

	while(i<list_size(entrenadores))
	{
		aux = (Jugador*)list_get(entrenadores,i);

		if(tiene_lo_que_pide(pivote,primero)&&(pivote != primero))
		{
			i = list_size(entrenadores);
		}

		else if(tiene_lo_que_pide(pivote,aux)&&(pivote != aux)&&(aux != primero))
		{
			pivote = aux;
			list_add(un_deadlock,aux);
			list_remove(entrenadores,i);
			i=0;
		}
		else i++;

	}

	return un_deadlock;

}

Pokemon* pokemon_mayor_nivel(Jugador* entrenador)
{
	Pokemon* mas_fuerte = (Pokemon*)list_get((entrenador->pokemonCapturados),0);

	int i=0;
	for(i=1;i<list_size(entrenador->pokemonCapturados);i++)
	{
		if((((Pokemon*)list_get(entrenador->pokemonCapturados,i))->pokemon->level)>(mas_fuerte->pokemon->level))
		{
			mas_fuerte = (Pokemon*)list_get(entrenador->pokemonCapturados,i);
		}
	}

	return mas_fuerte;
}

void resolver_deadlock(t_list* entrenadores)
{
	t_pokemon *poke2,*perdedor;

	t_pkmn_factory* fabrica = create_pkmn_factory();



	int cantidad_involucrados = list_size(entrenadores),i=0;

	Jugador *aux1,*aux2;

	Pokemon *mayor_nivel1,*mayor_nivel2;

	aux1 = (Jugador*)list_get(entrenadores,0);


	mayor_nivel1 = pokemon_mayor_nivel(aux1);

	perdedor = create_pokemon(fabrica, mayor_nivel1->nombre,mayor_nivel1->pokemon->level);


	for(i=1;i<cantidad_involucrados;i++)
	{
		aux2 = (Jugador*)list_get(entrenadores,i);

		mayor_nivel2 = pokemon_mayor_nivel(aux2);

		poke2 = create_pokemon(fabrica, mayor_nivel2->nombre,mayor_nivel2->pokemon->level);

		perdedor = pkmn_battle(perdedor,poke2);

	}
}


MetadataPokenest buscar_Pokenest(char simbolo,t_list* listaPokenest)
{
	bool _find_pokenest_(MetadataPokenest* aux)
	{
		return aux->simbolo == simbolo;
	}

	MetadataPokenest *ptr = (MetadataPokenest*) list_find(listaPokenest,(void*)_find_pokenest_);

	return *ptr;
}


int letra_pokenest(char *species)
{
	return tolower(species[0]);
}


int main(void)
{

	int i;
    t_list* pokenests = list_create();
    t_list* entrenadores = list_create();


    t_list* posibles_deadlock  = list_create();
    t_list* un_deadlock  = list_create();



    MetadataPokenest* pokenest = malloc(sizeof(MetadataPokenest));

    pokenest->cantPokemon = 0;
    pokenest->simbolo = 'p';

    list_add(pokenests,pokenest);

    pokenest = malloc(sizeof(MetadataPokenest));

    pokenest->cantPokemon = 0;
    pokenest->simbolo = 's';

    list_add(pokenests,pokenest);

    pokenest = malloc(sizeof(MetadataPokenest));

    pokenest->cantPokemon = 0;
    pokenest->simbolo = 'j';

    list_add(pokenests,pokenest);


    ///////////////////   UN ENTRENADOR


    Jugador* entrenador = malloc(sizeof(Jugador));

    entrenador->peticion = 's';

    entrenador->pokemonCapturados = list_create();
    Pokemon* pokeaux;

    pokeaux = malloc(sizeof(Pokemon));

    pokeaux->pokemon = malloc(sizeof(t_pokemon));

    pokeaux->nombre = string_new();

    strcpy((pokeaux->nombre),"Jigglypuff");

    pokeaux->pokemon->level = 1;

    list_add(entrenador->pokemonCapturados,pokeaux);

    list_add(entrenadores,entrenador);

    ///////////////////

    ///////////////////   OTRO ENTRENADOR



    entrenador = malloc(sizeof(Entrenador));

    entrenador->peticion = 'j';

    entrenador->pokemonCapturados = list_create();



    pokeaux = malloc(sizeof(Pokemon));

    pokeaux->pokemon = malloc(sizeof(t_pokemon));

    pokeaux->nombre = string_new();

    strcpy(pokeaux->nombre,"Pikachu");

    pokeaux->pokemon->level = 2;

    list_add(entrenador->pokemonCapturados,pokeaux);



    list_add(entrenadores,entrenador);


    ///////////////////

    ///////////////////   OTRO ENTRENADOR




    entrenador = malloc(sizeof(Entrenador));

    entrenador->peticion = 'p';

    entrenador->pokemonCapturados = list_create();

    pokeaux = malloc(sizeof(Pokemon));

    pokeaux->pokemon = malloc(sizeof(t_pokemon));

    pokeaux->nombre = string_new();

    strcpy(pokeaux->nombre,"Squirtle");

    pokeaux->pokemon->level = 9;

    list_add(entrenador->pokemonCapturados,pokeaux);

    list_add(entrenadores,entrenador);


    ///////////////////



    int** matriz_peticiones = generar_matriz_peticiones(entrenadores, pokenests);

    int** matriz_recursos_asignados = generar_matriz_asignados(entrenadores, pokenests);

    int* recursos_disponibles = generar_vector_recursos_disponibles(pokenests);

    posibles_deadlock = no_pueden_ejecutar(entrenadores,pokenests);


	printf("%d ", list_size(posibles_deadlock));
    if(list_size(posibles_deadlock)>1)
    {
    	un_deadlock = obtener_un_deadlock(posibles_deadlock);

    	printf("%d", list_size(un_deadlock));


    resolver_deadlock(un_deadlock);


    printf("                  PETICIONES\n");
    mostrar_matriz(matriz_peticiones,list_size(entrenadores),list_size(pokenests));
    printf("                  ASIGNADOS\n");
    mostrar_matriz(matriz_recursos_asignados,list_size(entrenadores),list_size(pokenests));

    mostrar_recursos_disponibles(recursos_disponibles, list_size(pokenests));
    }



    list_destroy(posibles_deadlock);

    list_destroy(un_deadlock);

    return 0;
}
