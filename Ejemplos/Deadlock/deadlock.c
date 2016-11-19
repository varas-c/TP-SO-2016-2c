#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <curses.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/config.h>
#include <commons/log.h>
#include <pkmn/battle.h>
#include <pkmn/factory.h>
#include <ctype.h>
#include "headers/struct.h"
#include "headers/deadlock.h"

Jugador* detectar_y_solucionar_deadlock(t_list* pokenests,t_list* entrenadores)
{

    t_list* posibles_deadlock  = list_create();

    t_list* entrenadores_aux=list_create();

    t_list* pokenests_aux=list_create();

    Jugador* perdedor =0;

	t_log* infoLogger = log_create("Logs.log", "Mapa", false, LOG_LEVEL_INFO);

    list_add_all(entrenadores_aux,entrenadores);

    list_add_all(pokenests_aux,pokenests);

	int** matriz_peticiones = inicializar_matriz(list_size(entrenadores), list_size(pokenests));

	int** matriz_recursos_asignados  = inicializar_matriz(list_size(entrenadores), list_size(pokenests));

	generar_matriz_peticiones(entrenadores_aux, pokenests_aux,matriz_peticiones);

	generar_matriz_asignados(entrenadores_aux, pokenests_aux,matriz_recursos_asignados);

	int* recursos_disponibles = generar_vector_recursos_disponibles(pokenests_aux);

	printf("                  PETICIONES\n");
	mostrar_matriz(matriz_peticiones,list_size(entrenadores_aux),list_size(pokenests_aux));

	log_info(infoLogger, "    PETICIONES");

	loggear_matriz(matriz_peticiones,pokenests_aux,entrenadores,infoLogger);

	printf("                  ASIGNADOS\n");
	mostrar_matriz(matriz_recursos_asignados,list_size(entrenadores_aux),list_size(pokenests_aux));

	log_info(infoLogger, "    ASIGNADOS");

	loggear_matriz(matriz_recursos_asignados,pokenests_aux,entrenadores,infoLogger);

	log_info(infoLogger, "    DISPONIBLES");

	loggear_vector(recursos_disponibles,pokenests_aux,infoLogger);

	mostrar_recursos_disponibles(recursos_disponibles, list_size(pokenests_aux));
	printf("\n\n\n");

		posibles_deadlock = no_pueden_ejecutar(entrenadores_aux,pokenests_aux,matriz_peticiones,matriz_recursos_asignados,recursos_disponibles);

		printf("Cantidad de entrenadores en deadlock o inanicion: %d\n\n",list_size(posibles_deadlock));

			sacar_inanicion(posibles_deadlock);
			if(list_size(posibles_deadlock)>1)
			{
				printf("Cantidad de entrenadores en algun deadlock: %d\n",list_size(posibles_deadlock));

				log_info(infoLogger, "    ENTRENADORES EN DEADLOCK");

				loggear_entrenadores_en_deadlock(posibles_deadlock,infoLogger);

				if(list_size(posibles_deadlock)>1)
				{
					perdedor = batalla_deadlock(posibles_deadlock,pokenests_aux);
				}
			}
			else
			{
				printf("No hay deadlock\n\n");
				log_info(infoLogger, "    NO HAY DEADLOCK");
			}



	printf("Muere el entrenador %d\n",perdedor);

	if(perdedor)
		printf("Y su numero es: %d\n",perdedor->numero);
		return perdedor;

		list_destroy(posibles_deadlock);

		list_destroy(entrenadores_aux);
}

int main(void)
{

    t_list* pokenests = list_create();
    t_list* entrenadores = list_create();

    MetadataPokenest* pokenest = malloc(sizeof(MetadataPokenest));

    pokenest->cantPokemon = 1;
    pokenest->simbolo = 'Z';

    list_add(pokenests,pokenest);

    pokenest = malloc(sizeof(MetadataPokenest));

    pokenest->cantPokemon = 0;
    pokenest->simbolo = 'B';

    list_add(pokenests,pokenest);

    pokenest = malloc(sizeof(MetadataPokenest));

    pokenest->cantPokemon = 1;
    pokenest->simbolo = 'G';

    list_add(pokenests,pokenest);

    pokenest = malloc(sizeof(MetadataPokenest));

    pokenest->cantPokemon = 0;
    pokenest->simbolo = 'P';

    list_add(pokenests,pokenest);

/*
    pokenest = malloc(sizeof(MetadataPokenest));

    pokenest->cantPokemon = 0;
    pokenest->simbolo = 'K';

    list_add(pokenests,pokenest);

    pokenest = malloc(sizeof(MetadataPokenest));

    pokenest->cantPokemon = 0;
    pokenest->simbolo = 'C';

    list_add(pokenests,pokenest);

    pokenest = malloc(sizeof(MetadataPokenest));

    pokenest->cantPokemon = 0;
    pokenest->simbolo = 'L';

    list_add(pokenests,pokenest);
*/  Pokemon* pokeaux;
    Jugador* entrenador;
    ///////////////////   UN ENTRENADOR

    entrenador = malloc(sizeof(Jugador));
    entrenador->peticion = 'P';
    entrenador->numero = 0;
    entrenador->pokemonCapturados = list_create();
    entrenador->entrenador.simbolo='#';
    pokeaux = malloc(sizeof(Pokemon));
    pokeaux->pokemon = malloc(sizeof(t_pokemon));
    pokeaux->nombre = string_new();
    strcpy(pokeaux->nombre,"B");
    pokeaux->pokemon->level = 2;
    pokeaux->pokenest='B';
    list_add(entrenador->pokemonCapturados,pokeaux);
    list_add(entrenadores,entrenador);

    ///////////////////   OTRO ENTRENADOR

    entrenador = malloc(sizeof(Jugador));
    entrenador->peticion = 'B';
    entrenador->numero = 1;
    entrenador->entrenador.simbolo='@';
    entrenador->pokemonCapturados = list_create();
    pokeaux;
    pokeaux = malloc(sizeof(Pokemon));
    pokeaux->pokemon = malloc(sizeof(t_pokemon));
    pokeaux->nombre = string_new();
    strcpy((pokeaux->nombre),"P");
    pokeaux->pokenest='P';
    pokeaux->pokemon->level = 1;
    list_add(entrenador->pokemonCapturados,pokeaux);
    list_add(entrenadores,entrenador);

    ///////////////////   OTRO ENTRENADOR

    entrenador = malloc(sizeof(Jugador));
    entrenador->peticion = 'Z';
    entrenador->numero = 2;
    entrenador->pokemonCapturados = list_create();
    entrenador->entrenador.simbolo='&';
    pokeaux = malloc(sizeof(Pokemon));
    pokeaux->pokemon = malloc(sizeof(t_pokemon));
    pokeaux->nombre = string_new();
    strcpy(pokeaux->nombre,"G");
    pokeaux->pokemon->level = 9;
    pokeaux->pokenest = 'G';
    list_add(entrenador->pokemonCapturados,pokeaux);
    list_add(entrenadores,entrenador);

    ///////////////////   OTRO ENTRENADOR

    entrenador = malloc(sizeof(Jugador));
    entrenador->peticion = 'G';
    entrenador->numero = 2;
    entrenador->pokemonCapturados = list_create();
    entrenador->entrenador.simbolo=')';
    /*
    pokeaux = malloc(sizeof(Pokemon));
    pokeaux->pokemon = malloc(sizeof(t_pokemon));
    pokeaux->nombre = string_new();
    strcpy(pokeaux->nombre,"G");
    pokeaux->pokemon->level = 9;
    pokeaux->pokenest = 'G';
    list_add(entrenador->pokemonCapturados,pokeaux);
    */
    list_add(entrenadores,entrenador);

/*
    entrenador = malloc(sizeof(Jugador));

    entrenador->peticion = 'Z';

    entrenador->numero = 3;

    entrenador->pokemonCapturados = list_create();

    entrenador->entrenador.simbolo='&';

    pokeaux = malloc(sizeof(Pokemon));

    pokeaux->pokemon = malloc(sizeof(t_pokemon));

    pokeaux->nombre = string_new();

    strcpy(pokeaux->nombre,"Bapdos");

    pokeaux->pokemon->level = 9;

    pokeaux->pokenest = 'B';

    list_add(entrenador->pokemonCapturados,pokeaux);

    list_add(entrenadores,entrenador);



    ///////////////////


     entrenador = malloc(sizeof(Jugador));

     entrenador->peticion = 'B';

     entrenador->numero = 4;

     entrenador->pokemonCapturados = list_create();

     entrenador->entrenador.simbolo='@';

     pokeaux = malloc(sizeof(Pokemon));

     pokeaux->pokemon = malloc(sizeof(t_pokemon));

     pokeaux->nombre = string_new();

     strcpy(pokeaux->nombre,"Krabby");

     pokeaux->pokenest='K';

     pokeaux->pokemon->level = 9;

     list_add(entrenador->pokemonCapturados,pokeaux);

     list_add(entrenadores,entrenador);

     ///////////////////



     entrenador = malloc(sizeof(Jugador));

      entrenador->peticion = 'C';

      entrenador->numero = 5;

      entrenador->pokemonCapturados = list_create();

      entrenador->entrenador.simbolo='*';

      pokeaux = malloc(sizeof(Pokemon));

      pokeaux->pokemon = malloc(sizeof(t_pokemon));

      pokeaux->pokemon->species = string_new();

      strcpy(pokeaux->pokemon->species,"Lickitung");

      pokeaux->pokenest='L';

      pokeaux->pokemon->level = 9;

      list_add(entrenador->pokemonCapturados,pokeaux);

      list_add(entrenadores,entrenador);

      ///////////////////

      entrenador = malloc(sizeof(Jugador));

       entrenador->peticion = 'L';

       entrenador->numero = 6;

       entrenador->pokemonCapturados = list_create();

       entrenador->entrenador.simbolo='+';

       pokeaux = malloc(sizeof(Pokemon));

       pokeaux->pokemon = malloc(sizeof(t_pokemon));

       pokeaux->pokemon->species = string_new();

       strcpy(pokeaux->pokemon->species,"Charizard");

       pokeaux->pokenest='C';

       pokeaux->pokemon->level = 9;

       list_add(entrenador->pokemonCapturados,pokeaux);

       list_add(entrenadores,entrenador);

       */

       ///////////////////

       t_log * log = log_create("LogDeadlock.log", "Mapa", false, LOG_LEVEL_INFO);

       t_list* entrenadores_aux = list_create();

       entrenadores_aux = obtener_deadlock(pokenests,entrenadores,log);

       list_destroy(entrenadores_aux);

       log_destroy(log);

       list_destroy(entrenadores);

       list_destroy(pokenests);


    return 0;
}
