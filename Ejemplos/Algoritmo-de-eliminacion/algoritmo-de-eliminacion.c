#include <stdio.h>
#include <tad_items.h>
#include <string.h>
#include <stdlib.h>
#include <commons/config.h>
#include <sys/ioctl.h>
#include <curses.h>
#include <nivel.h>
#include <commons/collections/list.h>
#include <commons/log.h>
#include <commons/collections/queue.h>
#include <math.h>
#include <ctype.h>

typedef struct{
	char simbolo;
	int posx;
	int posy;
	char movAnterior;
	int flagx;
	int flagy;
}Entrenador;

typedef struct
{
	Entrenador entrenador;
	int estado;
	int socket;
}Jugador;

void inicializar_entrenador(Entrenador* entrenador)
{
    entrenador->posx = 1;
    entrenador->posy = 1;
    entrenador->simbolo = '@';
    entrenador->movAnterior = 'y';
    entrenador -> flagx = FALSE;
    entrenador -> flagy = FALSE;
}

void inicializar_jugador(Jugador* unJugador, int unSocket){
	inicializar_entrenador(&(unJugador->entrenador));
	unJugador->socket = unSocket;
	unJugador->estado = 0;
}

void eliminar_elemento(t_queue* cola, Jugador *element){
	t_queue* cola_auxiliar = queue_create();
	Jugador *auxiliar, *auxiliar2;
	int tamanio = queue_size(cola);
	int tamanio2;
	while(tamanio > 0){
		auxiliar = queue_pop(cola);
		if(auxiliar != element){
			queue_push(cola_auxiliar, auxiliar);
		}
		tamanio = tamanio-1;
	}
	tamanio2 = queue_size(cola_auxiliar);
	while(tamanio2 > 0){
		auxiliar2 = queue_pop(cola_auxiliar);
		queue_push(cola, auxiliar2);
		tamanio2 = tamanio2 - 1;
	}
}

int main(int argc, char** argv){
	t_queue* cola = queue_create();
	Jugador a,b,c;
	Jugador *res1, *res2;
	int simbolo1, simbolo2;
	a.entrenador.simbolo = '@';
	b.entrenador.simbolo = '$';
	c.entrenador.simbolo = '%';
	queue_push(cola, &a);
	queue_push(cola, &b);
	queue_push(cola, &c);
	eliminar_elemento(cola,&a);
	eliminar_elemento(cola,&c);
	res1 = queue_pop(cola);
	simbolo1 = res1->entrenador.simbolo;
	//res2 = queue_pop(cola);
	//simbolo2 = res2->entrenador.simbolo;
	printf("%c\n", simbolo1);
	//printf("%c\n", simbolo2);
	return 0;
}
