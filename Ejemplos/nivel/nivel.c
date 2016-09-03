#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <tad_items.h>
#include <stdlib.h>
#include <curses.h>
#include <commons/collections/list.h>
#include "nivel.h"



/*
 * @NAME: rnd
 * @DESC: Modifica el numero en +1,0,-1, sin pasarse del maximo dado
 */
void rnd(int *x, int max){
	*x += (rand() % 3) - 1;
	*x = (*x<max) ? *x : max-1;
	*x = (*x>0) ? *x : 1;
}

typedef struct
{
	char simbolo;
	int posx;
	int posy;
	int cant;
}Pokenest;

typedef struct{
	char simbolo;
	int posx;
	int posy;
	char movAnterior;
	int flagx;
	int flagy;
	int destinox;
	int destinoy;
}Entrenador;


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


int main(void) {
    t_list* items = list_create(); //Lista donde se almacenan los items

    Pokenest pokenest;
    Pokenest pokenest2;
    Entrenador entrenador;

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

	CrearPersonaje(items, '@',entrenador.posx, entrenador.posy);


	CrearCaja(items, pokenest.simbolo, pokenest.posx, pokenest.posy, pokenest.cant);
	CrearCaja(items, pokenest2.simbolo, pokenest2.posx, pokenest2.posy, pokenest2.cant);

	char mostrar[20];
	sprintf(mostrar,"X:%i -- Y:%i",entrenador.posx,entrenador.posy);
	nivel_gui_dibujar(items, mostrar);

	int flag = 0;

	calcular_coordenadas(&entrenador,pokenest.posx,pokenest.posy);

	while ( 1 ) {
		int key = getch();

		if(flag==0){

		mover_entrenador(&entrenador);
		MoverPersonaje(items, entrenador.simbolo, entrenador.posx, entrenador.posy);

		if (entrenador.posx == pokenest.posx && entrenador.posy ==pokenest.posy) {
			restarRecurso(items, pokenest.simbolo);

			if(pokenest.cant > 0)
			pokenest.cant-=1;
		}

		if(pokenest.cant == 0)
		{
			flag = 1;
			entrenador.movAnterior = 'y';
			calcular_coordenadas(&entrenador,pokenest2.posx,pokenest2.posy);
		}

		}



		if (flag==1) {

		mover_entrenador(&entrenador);
		MoverPersonaje(items, entrenador.simbolo, entrenador.posx, entrenador.posy);

		if (entrenador.posx == pokenest2.posx && entrenador.posy ==pokenest2.posy) {
			//restarRecurso(items, pokenest2.simbolo);
			BorrarItem(items, pokenest2.simbolo);
		}

		}

		char buffer[20];
		sprintf(buffer,"Y:%i",entrenador.posx,entrenador.posy);
		nivel_gui_dibujar(items, buffer);


	}


	BorrarItem(items, '@');
	BorrarItem(items, 'Z');


	nivel_gui_terminar();

}
