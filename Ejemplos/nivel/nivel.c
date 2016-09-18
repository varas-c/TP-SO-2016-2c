#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <tad_items.h>
#include <stdlib.h>
#include <curses.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <pkmn/battle.h>
#include <pkmn/factory.h>
#include "struct.h"
#include "nivel.h"
#include "configMapa.h"

/*
 * @NAME: rnd
 * @DESC: Modifica el numero en +1,0,-1, sin pasarse del maximo dado
 */
void rnd(int *x, int max){
	*x += (rand() % 3) - 1;
	*x = (*x<max) ? *x : max-1;
	*x = (*x>0) ? *x : 1;
}

//Si flagx=false then mover en X otherwise mover en Y
void mover_entrenador(Entrenador *entrenador)
{
	int move = 0;

	if(entrenador->destinox != 0 && entrenador->movAnterior == 'y' && move == 0)
	{
		if(entrenador->destinox < 0 ) //Me muevo hacia atras en X
		{
			entrenador->posx -=1;
			entrenador->destinox += 1;
		}

		if(entrenador->destinox > 0)
		{
			entrenador->posx += 1;
			entrenador->destinox -= 1;
		}

		entrenador->movAnterior = 'x';
		move = 1;
	}

	if(entrenador->destinoy != 0 && entrenador->movAnterior == 'x' && move == 0)
	{
		if(entrenador->destinoy < 0 ) //Me muevo hacia atras en Y
		{
			entrenador->posy -=1;
			entrenador->destinoy += 1;
		}

		if(entrenador->destinoy > 0)
		{
			entrenador->posy += 1;
			entrenador->destinoy -= 1;
		}

		entrenador->movAnterior = 'y';
		move = 1;
	}

	if(entrenador->destinox != 0 && move == 0) //Si todavia me queda mvimiento en X
	{
		if(entrenador->destinox < 0 ) //Me muevo hacia atras en X
				{
					entrenador->posx -=1;
					entrenador->destinox += 1;
				}

				if(entrenador->destinox > 0)
				{
					entrenador->posx += 1;
					entrenador->destinox -= 1;
				}

				entrenador->movAnterior = 'x';
				move = 1;
	}

	if(entrenador->destinoy != 0 && move == 0)
	{
		if(entrenador->destinoy < 0 ) //Me muevo hacia atras en X
		{
			entrenador->posy -=1;
			entrenador->destinoy += 1;
		}

		if(entrenador->destinoy > 0)
		{
			entrenador->posy += 1;
			entrenador->destinoy -= 1;
		}

		entrenador->movAnterior = 'y';
		move = 1;
	}

}





void inicializar_entrenador(Entrenador* entrenador)
{
    entrenador->posx = 1;
    entrenador->posy = 1;
    entrenador->simbolo = '@';
    entrenador->movAnterior = 'y';
    entrenador -> flagx = FALSE;
    entrenador -> flagy = FALSE;
}


void calcular_coordenadas(Entrenador* entrenador, int x, int y)
{
	//Pregunto por X

	if(x > entrenador->posx)
	{
		entrenador ->destinox = x - entrenador->posx;
	}

	if(entrenador->posx == x)
	{
		entrenador ->destinox = 0;
	}

	if(x < entrenador->posx)
	{
		entrenador->destinox = x - entrenador->posx;
	}

	//Pregunto por y

	if(y > entrenador->posy)
	{
		entrenador ->destinoy = y - entrenador->posy;
	}

	if(entrenador->posy == y)
	{
		entrenador ->destinoy = 0;
	}

	if(y < entrenador->posy)
	{
		entrenador->destinoy = y - entrenador->posy;
	}

}

void cargarPokenests(t_list* pokenests, t_list* items)
{
	int i=0;
	Pokenest* aux;
	while(aux=list_get(pokenests,i))
	{
		CrearCaja(items, aux->simbolo, aux->posx, aux->posy, aux->cant);
		i++;
	}
}

ITEM_NIVEL* obtener_pokenest_por_simbolo(t_list* items, char id) {

    bool _search_by_id(ITEM_NIVEL* item) {
        return item->id == id;
    }

    return list_find(items, (void*) _search_by_id);
}

void interactuar(Entrenador* entrenador,t_list* items,char* objetivos,t_list* pokenests)
{
	int i=0,key;
	char mostrar[20];

	char proximo_simbolo;
	ITEM_NIVEL* pokeaux;

	sprintf(mostrar,"X:%i -- Y:%i",entrenador->posx,entrenador->posy);
	nivel_gui_dibujar(items, mostrar);

	while((pokeaux=obtener_pokenest_por_simbolo(pokenests,objetivos[i]))!=NULL)
	{
			calcular_coordenadas(entrenador,pokeaux->posx,pokeaux->posy);

			while(!(entrenador->posx == pokeaux->posx && entrenador->posy ==pokeaux->posy))
			{
				key = getch();
				mover_entrenador(entrenador);
				MoverPersonaje(items, entrenador->simbolo, entrenador->posx, entrenador->posy);
				nivel_gui_dibujar(items, mostrar);
			}
			restarRecurso(items, pokeaux->id);
			i+=2;
			nivel_gui_dibujar(items, mostrar);
	}

}

t_pokemon generarPokemon(MetadataPokemon* mdata,t_pkmn_factory* fabrica, char* especie)
{
	return *create_pokemon(fabrica, especie, mdata->nivel);
}

int main(void) {
	/*

    t_list* items = list_create(); //Lista donde se almacenan los items
    t_list* pokenests = list_create();
    Pokenest pokenest;
    Pokenest pokenest2;
    Pokenest pokenest3;
    Entrenador entrenador;

    char objetivos[4][2];
    strcpy(objetivos[0],"K");
    strcpy(objetivos[1],"Z");
    strcpy(objetivos[2],"J");
    strcpy(objetivos[3],"'\0'");

	//Inicializamos espacio de dibujo
	nivel_gui_inicializar();

    inicializar_entrenador(&entrenador);

    pokenest.posx = 40;
    pokenest.posy = 5;
    pokenest.cant = 5;
    pokenest.simbolo = 'Z';

    pokenest2.posx=5;
    pokenest2.posy=11;
    pokenest2.cant = 3;
    pokenest2.simbolo = 'K';

    pokenest3.posx=25;
    pokenest3.posy=11;
    pokenest3.cant = 3;
    pokenest3.simbolo = 'J'; //a esta ultima no va a ir

	CrearPersonaje(items, '@',entrenador.posx, entrenador.posy);

	list_add(pokenests,&pokenest);
	list_add(pokenests,&pokenest2);
	list_add(pokenests,&pokenest3);
	cargarPokenests(pokenests,items);

	interactuar(&entrenador,items,objetivos,pokenests);

	BorrarItem(items, '@');
	BorrarItem(items, 'Z');

	nivel_gui_terminar();
	*/

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

	return 0;
}
