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
#include <commons/config.h>
#include <tad_items.h>

typedef struct{
	char simbolo;
	int nivel;
}PokemonDL;

typedef struct{
	char simbolo;
	t_list* recursos;
	char peticion;
}EntrenadorDL;

typedef struct{
	char simbolo;
	int cant;
}PokenestDL;





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


int cantidad_obtenidos_de_un_tipo(EntrenadorDL* entrenador,char simbolo)//CANT DE POKEMONES ASIGNADOS A ESE ENTRENADOR DE UNA POKENEST
{
	int i=0,cantidad_total,cantidad=0;
	PokemonDL* pokemon_aux;
	cantidad_total = list_size(entrenador->recursos);
	while(i<cantidad_total)
	{
		pokemon_aux=(PokemonDL*)list_get(entrenador->recursos,i);
		if(pokemon_aux->simbolo==simbolo) //SI ESE POKEMON ES DE ESA POKENEST
		{
			cantidad++;
		}
		i++;
	}
	return cantidad;
}

int** generar_matriz_peticiones(t_list* entrenadores, t_list* pokenests)
{
	EntrenadorDL* entrenador_aux;
	PokenestDL* pokenest_aux;
	int i,j,cant_pokenests,cant_entrenadores,cantidad_recursos;

	cant_pokenests=list_size(pokenests);
	cant_entrenadores=list_size(entrenadores);

	int** matriz= inicializar_matriz(cant_entrenadores, cant_pokenests);//DEVUELVE MATRIZ CON TODOS 0


	i = 0;

	while(i<cant_pokenests)
	{
		pokenest_aux = (PokenestDL*)list_get(pokenests,i);
		j = 0;
		while(j<cant_entrenadores)
		{
			entrenador_aux = (EntrenadorDL*)list_get(entrenadores,j);
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
	EntrenadorDL* entrenador_aux;
	PokenestDL* pokenest_aux;
	int i=0,j=0,cant_pokenests,cant_entrenadores,cantidad_recursos;

	cant_pokenests=list_size(pokenests);

	cant_entrenadores=list_size(entrenadores);


	int** matriz= inicializar_matriz(cant_entrenadores, cant_pokenests);//DEVUELVE MATRIZ CON TODOS 0

	while(i<cant_pokenests)
	{
		cantidad_recursos = 0;
		pokenest_aux = (PokenestDL*)list_get(pokenests,i);
		j=0;
		while(j<cant_entrenadores)
		{
			entrenador_aux = (EntrenadorDL*)list_get(entrenadores,j);
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
		vector[i]=((PokenestDL*)list_get(pokenests,i))->cant;
		i++;
	}
	return vector;
}

t_list * detectar_deadlock(t_list* entrenadores, t_list*pokenests)
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
					j=pokenests;   //    ESTE ENTRENADOR PUEDE OBTENER EL POKEMON DE LA POKENEST
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


bool tiene_lo_que_pide(EntrenadorDL* entrenador, EntrenadorDL* otro_entrenador)
{
	int i,j;
	PokemonDL* pokeaux;
	for(j=0;j<list_size(otro_entrenador->recursos);j++)
	{
		if(entrenador->peticion==*(char*)list_get(otro_entrenador->recursos,j))
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

	EntrenadorDL *aux1,*aux2,*primero;

	int cantidad,i=0;

	cantidad = list_size(entrenadores);

	primero = ((EntrenadorDL*)list_get(entrenadores,0));//COMO TODOS ESTAN EN DEADLOCK, AGARRO UNO Y VEO CON CUALES ESTA EN DEADLOCK

	list_add(un_deadlock,primero);

	for(i=1;i<cantidad;i++)
	{
		aux1 = (EntrenadorDL*)list_get(entrenadores,i-1);
		aux2 = (EntrenadorDL*)list_get(entrenadores,i);
		if(tiene_lo_que_pide(aux1,aux2))
		{
			list_add(un_deadlock,aux2);
		}
		if(tiene_lo_que_pide(aux2,primero))
		{
			i = cantidad;
		}
	}

	return un_deadlock;

}

bool resolver_deadlock(t_list* entrenadores)
{
	if(list_size(entrenadores))
	{
		//BATALLAS...
	}
}

PokemonDL* pokemon_mas_fuerte(EntrenadorDL* entrenador)
{
	PokemonDL* mas_fuerte = (PokemonDL*)list_get(entrenador->recursos,0);
	int i=0;
	for(i=0;i<list_size(entrenador->recursos);i++)
	{
		if(((PokemonDL*)list_get(entrenador->recursos,i))->nivel>mas_fuerte->nivel)
		{
			mas_fuerte = list_get(entrenador->recursos,i);
		}
	}
	return mas_fuerte;
}

int main(void)
{
	int i;
    t_list* pokenests = list_create();
    t_list* entrenadores = list_create();
    t_list* posibles_deadlock;
    t_list* un_deadlock;

    PokenestDL* pokenest1 = malloc(sizeof(PokenestDL));

    pokenest1->cant = 0;
    pokenest1->simbolo = 'p';

    PokenestDL* pokenest2 = malloc(sizeof(PokenestDL));

    pokenest2->cant = 0;
    pokenest2->simbolo = 's';

    PokenestDL* pokenest3 = malloc(sizeof(PokenestDL));

    pokenest3->cant = 0;
    pokenest3->simbolo = 'j';

    list_add(pokenests,pokenest1);

    list_add(pokenests,pokenest2);

    //list_add(pokenests,pokenest3);

    ///////////////////   UN ENTRENADOR

    EntrenadorDL* entrenador = malloc(sizeof(EntrenadorDL));

    entrenador->peticion = 's';

    entrenador->recursos = list_create();

    PokemonDL* pokeaux = malloc(sizeof(PokemonDL));
    pokeaux->simbolo = 's';

    list_add(entrenador->recursos,pokeaux);

    pokeaux = malloc(sizeof(PokemonDL));
    pokeaux->simbolo = 'p';

    list_add(entrenador->recursos,pokeaux);

    list_add(entrenadores,entrenador);

    ///////////////////

    ///////////////////   OTRO ENTRENADOR

    entrenador = malloc(sizeof(EntrenadorDL));

    entrenador->peticion = 'p';

    entrenador->recursos = list_create();

    pokeaux = malloc(sizeof(PokemonDL));
    pokeaux->simbolo = 'p';

    list_add(entrenador->recursos,pokeaux);

    pokeaux = malloc(sizeof(PokemonDL));
    pokeaux->simbolo = 'p';

    list_add(entrenador->recursos,pokeaux);

    list_add(entrenadores,entrenador);


    ///////////////////

    ///////////////////   OTRO ENTRENADOR

    entrenador = malloc(sizeof(EntrenadorDL));

    entrenador->peticion = 'p';

    entrenador->recursos = list_create();

    pokeaux = malloc(sizeof(PokemonDL));
    pokeaux->simbolo = 'p';

    list_add(entrenador->recursos,pokeaux);

    pokeaux = malloc(sizeof(PokemonDL));
    pokeaux->simbolo = 'p';

    list_add(entrenador->recursos,pokeaux);

    pokeaux = malloc(sizeof(PokemonDL));
    pokeaux->simbolo = 'p';

    list_add(entrenador->recursos,pokeaux);

    pokeaux = malloc(sizeof(PokemonDL));
    pokeaux->simbolo = 's';

    list_add(entrenador->recursos,pokeaux);

    pokeaux = malloc(sizeof(PokemonDL));
    pokeaux->simbolo = 's';

    list_add(entrenador->recursos,pokeaux);

    list_add(entrenadores,entrenador);


    ///////////////////

    int** matriz_peticiones = generar_matriz_peticiones(entrenadores, pokenests);

    int** matriz_recursos_asignados = generar_matriz_asignados(entrenadores, pokenests);


    int* recursos_disponibles = generar_vector_recursos_disponibles(pokenests);

    posibles_deadlock = detectar_deadlock(entrenadores,pokenests);


    un_deadlock = obtener_un_deadlock(posibles_deadlock);

    printf("                  PETICIONES\n");
    mostrar_matriz(matriz_peticiones,list_size(entrenadores),list_size(pokenests));
    printf("                  ASIGNADOS\n");
    mostrar_matriz(matriz_recursos_asignados,list_size(entrenadores),list_size(pokenests));

    mostrar_recursos_disponibles(recursos_disponibles, list_size(pokenests));

    list_destroy(posibles_deadlock);

    list_destroy(un_deadlock);

    return 0;
}
