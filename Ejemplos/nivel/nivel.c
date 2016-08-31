
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
}Entrenador;


//Si flagx=false then mover en X otherwise mover en Y
void mover_entrenador(Entrenador *entrenador, int destinox, int destinoy)
{
	//Comenzamos de 1,1

	int movimiento = FALSE; //Es para saber si en esta llamada a la funcion, realicé un movimiento

	if(entrenador->movAnterior == 'y' && movimiento == FALSE) //Si el movimiento anterior fue en Y y todavía no me movi, me teno que mover en X
	{
		if(entrenador->flagx == FALSE) //Si no llegué al maximo de X
		{
			if(entrenador->posx != destinox) //Si no llegué al DestinoX
			{
				entrenador->posx +=1; //Me muevo 1 en X
				entrenador -> movAnterior = 'x'; //Me movi en X, asigno asi se que el siguient movimiento tiene que ser en Y
				movimiento = TRUE; //Ya me moví en este turno, asi que no debería moverme
			}

			else
			{
				entrenador->flagx = TRUE; //Si llegué al destino de X, pongo una bandera asi ya no me muevo mas en X
			}
		}
	}


	if(entrenador->movAnterior == 'x' && movimiento == FALSE) //Si el movimiento anterior fue en X y todavía no me movi
	{
		if(entrenador->flagy == FALSE) //Si todavia no llegué al maximo de Y
		{
			if(entrenador->posy != destinoy) //Si todavia no llegue a DestinoY
			{
				entrenador->posy +=1;//me muevo 1
				entrenador -> movAnterior = 'y';//indico que me movi en Y, asi el proximo movimiento es en X
				movimiento = TRUE; //Este turno ya me moví, asi que no debería moverme
			}

			else
			{
				entrenador->flagy = TRUE; //Si llegué al destino de Y, pongo una bandera asi ya no me muevo mas en Y
			}
		}
	}

	//Si ya llegué al X,Y maximo pero todavia no llegué a destino

	//llegue en X pero todavia no en Y
	if(entrenador->posx == destinox && entrenador->posy != destinoy && movimiento != TRUE)
	{
		entrenador->posy +=1;
		movimiento = TRUE;
	}

	//Llegue al destinoY pero todavía no en X
	if(entrenador->posy == destinoy && entrenador->posx != destinox && movimiento != TRUE)
	{
			entrenador->posx +=1;
	}




	/*
	if(entrenador->flagx==FALSE)
	{
		entrenador->posx +=1;
		entrenador->flagx = TRUE;

	}

	else
	{
		entrenador->posy += 1;
		entrenador->flagx = FALSE;
	}
	*/


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


int main(void) {
    t_list* items = list_create(); //Lista donde se almacenan los items

    Pokenest pokenest;
    Entrenador entrenador;

	int rows, cols; //tamaño de la pantalla


	//Inicializamos espacio de dibujo
	nivel_gui_inicializar();

	//Obtenemos el area de la consola
    nivel_gui_get_area_nivel(&rows, &cols);

    inicializar_entrenador(&entrenador);

    pokenest.posx = 5;
    pokenest.posy = 19;
    pokenest.cant = 5;
    pokenest.simbolo = 'Z';


	CrearPersonaje(items, '@',entrenador.posx, entrenador.posy);


	CrearCaja(items, pokenest.simbolo, pokenest.posx, pokenest.posy, pokenest.cant);

	char mostrar[20];
	sprintf(mostrar,"X:%i -- Y:%i",entrenador.posx,entrenador.posy);
	nivel_gui_dibujar(items, mostrar);


	while ( 1 ) {
		int key = getch();

		mover_entrenador(&entrenador, pokenest.posx,pokenest.posy);
		MoverPersonaje(items, entrenador.simbolo, entrenador.posx, entrenador.posy);


		if (entrenador.posx == pokenest.posx && entrenador.posy ==pokenest.posy) {
			restarRecurso(items, pokenest.simbolo);
		}


		char buffer[20];
		sprintf(buffer,"X:%i -- Y:%i",entrenador.posx,entrenador.posy);
		nivel_gui_dibujar(items, buffer);


	}


	BorrarItem(items, '@');
	BorrarItem(items, 'Z');


	nivel_gui_terminar();

}
