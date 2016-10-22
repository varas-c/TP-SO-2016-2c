/*
 * deadlock.h
 *
 *  Created on: 14/10/2016
 *      Author: utnso
 */

#ifndef HEADERS_DEADLOCK_H_
#define HEADERS_DEADLOCK_H_

//*************************************************************
//*************************************************************
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

//*************************************************************
//*************************************************************

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

//*************************************************************
//*************************************************************

void mostrar_recursos_disponibles(int* recursos, int cant_recursos)
{
	int i=0;
	printf("RECURSOS          ");
	for (i=0;i<cant_recursos;i++)
	{
		printf("%d  ", recursos[i]);
	}
}

//*************************************************************
//*************************************************************

int cantidad_obtenidos_de_un_tipo(Jugador* entrenador,char simbolo)//CANT DE POKEMONES ASIGNADOS A ESE ENTRENADOR DE UNA POKENEST
{
	int i=0,cantidad_total,cantidad=0;
	Pokemon* pokemon_aux;
	cantidad_total = list_size(entrenador->pokemonCapturados);
	while(i<cantidad_total)
	{
		pokemon_aux=(Pokemon*)list_get(entrenador->pokemonCapturados,i);
		if(letra_pokenest(pokemon_aux->pokemon->species)==simbolo) //SI ESE POKEMON ES DE ESA POKENEST
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

	while(i<cant_entrenadores)
	{
		entrenador_aux = (Jugador*)list_get(entrenadores,i);
		j = 0;
		while(j<cant_pokenests)
		{
			pokenest_aux = (MetadataPokenest*)list_get(pokenests,j);
			if(entrenador_aux->peticion == pokenest_aux->simbolo)
			{
				matriz[i][j]=1;
				j=cant_pokenests;
			}

				j++;
		}
		i++;
	}
	return matriz;
}

//*************************************************************
//*************************************************************

int** generar_matriz_asignados(t_list* entrenadores, t_list* pokenests)
{
	Jugador* entrenador_aux;
	MetadataPokenest* pokenest_aux;
	int i=0,j=0,cant_pokenests,cant_entrenadores,cantidad_recursos;

	cant_pokenests=list_size(pokenests);

	cant_entrenadores=list_size(entrenadores);

	int** matriz= inicializar_matriz(cant_entrenadores, cant_pokenests);//DEVUELVE MATRIZ CON TODOS 0

	while(i<cant_entrenadores)
	{
		cantidad_recursos = 0;
		entrenador_aux = (Jugador*)list_get(entrenadores,i);

		j=0;
		while(j<cant_pokenests)
		{
			pokenest_aux = (MetadataPokenest*)list_get(pokenests,j);
			cantidad_recursos = cantidad_obtenidos_de_un_tipo(entrenador_aux, pokenest_aux->simbolo); //CANT DE POKEMONES ASIGNADOS A ESE ENTRENADOR DE ESA POKENEST
			matriz[i][j]=cantidad_recursos;
			j++;
		}
		i++;
	}
	return matriz;
}

//*************************************************************
//*************************************************************

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

//*************************************************************
//*************************************************************

t_list* no_pueden_ejecutar(t_list* entrenadores, t_list*pokenests)
{
	int **matriz_peticiones,**matriz_recursos_asignados,*recursos_disponibles,cant_entrenadores, cant_pokenests;
	bool tiene_recursos = false,entrenador_satisfecho = false;
	cant_entrenadores = list_size(entrenadores);
	cant_pokenests = list_size(pokenests);
	int posible_deadlock[cant_entrenadores];
	int i,j,k;

	matriz_peticiones = generar_matriz_peticiones(entrenadores, pokenests);

	matriz_recursos_asignados = generar_matriz_asignados(entrenadores,pokenests);

	recursos_disponibles = generar_vector_recursos_disponibles(pokenests);

	t_list* no_puede_ejecutar = list_create();

    for(i=0;i<cant_entrenadores;i++)
    {
    	posible_deadlock[i]=1;
    }

    //VEMOS QUIEN NO TIENE RECURSOS

    for(i=0;i<cant_entrenadores;i++)  //RECORRO CADA ENTRENADOR
	{
		tiene_recursos = false;
		for(j=0;j<cant_pokenests;j++)  //RECORRO CADA POKENEST (RECURSO)
		{
			if(matriz_recursos_asignados[i][j])
				tiene_recursos=true;
		}

		if(!tiene_recursos)
			posible_deadlock[i]=0;
	}

    for(i=0;i<cant_entrenadores;i++)
	{
		if(posible_deadlock[i])
		{
			entrenador_satisfecho = true;
			for(j=0;j<cant_pokenests;j++)
			{
				if((matriz_peticiones[i][j] > recursos_disponibles[j])&&(entrenador_satisfecho))
				{
					entrenador_satisfecho = false;
				}

			}
			if(entrenador_satisfecho)
			{
				posible_deadlock[i] = 0;
				for(k=0;k<cant_pokenests;k++)  //NO ESTA EN DEADLOCK, "DEVUELVE" SUS RECURSOS
				{
					recursos_disponibles[k] += matriz_recursos_asignados[i][k];
				}
				j=cant_pokenests;
				i=0;
			}
		}
	}


	for(i=0;i<cant_entrenadores;i++)
	{
		if(posible_deadlock[i])
		{
			list_add(no_puede_ejecutar,list_get(entrenadores,i)); // DEVUELVO LISTA CON LOS QUE PUEDEN ESTAR EN DL
		}
	}


	return no_puede_ejecutar; //DEVUELVO LOS QUE NO PUEDEN EJECUTAR, FALTA EVALUAR LA ESPERA CIRCULAR
}

//*************************************************************
//*************************************************************

int letra_pokenest(char *species)
{
	return tolower(species[0]);
}


//*************************************************************
//*************************************************************

bool tiene_lo_que_pide(Jugador* entrenador, Jugador* otro_entrenador)
{
	int i;
	Pokemon* poke1;
	if(entrenador!=otro_entrenador)

		for(i=0;i<list_size(otro_entrenador->pokemonCapturados);i++)
		{
			poke1=(Pokemon*)list_get(otro_entrenador->pokemonCapturados,i);

			if(((letra_pokenest(poke1->pokemon->species))==entrenador->peticion))
			{
				return true;
			}
		}
	else return false;

	return false;
}


//*************************************************************
//*************************************************************

bool esta_en_inanicion(Jugador* entrenador,t_list* entrenadores)
{
	int j;
	bool inanicion = true;

	if(list_size(entrenadores)<=1)
	{
		return false;
	}

	for(j=0;j<list_size(entrenadores);j++)
	{
		if(tiene_lo_que_pide(list_get(entrenadores,j),entrenador))
		{
			inanicion=false;
			return inanicion;
		}
	}
	return inanicion;
}



//*************************************************************
//*************************************************************

bool sacar_inanicion(t_list* entrenadores)
{
	bool se_encontro=false;
	int i;
	Jugador* aux;


	for(i=0;i<list_size(entrenadores);i++)
	{
		aux=list_get(entrenadores,i);
		if(esta_en_inanicion(aux,entrenadores))
		{
			list_remove(entrenadores,i);
			se_encontro = true;
			i=0;
		}
	}


	return se_encontro;
}


//*************************************************************
//*************************************************************

t_pokemon* pokemon_mayor_nivel(t_list* pokemones)
{
	t_pokemon* mas_fuerte = (t_pokemon*)list_get((pokemones),0);

	int i=0;
	for(i=1;i<list_size(pokemones);i++)
	{
		if((((t_pokemon*)list_get(pokemones,i))->level)>(mas_fuerte->level))
		{
			mas_fuerte = (Pokemon*)list_get(pokemones,i);
		}
	}

	return mas_fuerte;
}
//*************************************************************

Jugador* entrenador_tiene_pokemon(t_list* entrenadores,t_pokemon* poke)
{
	int i,j;
	Jugador* entrenador_aux;
	Pokemon* poke_aux;

	for(i=0;i<list_size(entrenadores);i++)
	{
		entrenador_aux = list_get(entrenadores,i);
		for(j=0;j<list_size(entrenador_aux->pokemonCapturados);j++)
		{
			poke_aux = (Pokemon*)list_get(entrenador_aux->pokemonCapturados,j);
			if(poke_aux->pokemon == poke)
			{
				return entrenador_aux;
			}
		}
	}
	return 0;
}

/*
void liberar_recursos_entrenador(Jugador* entrenador_perdedor,t_list* pokenests)
{
	int i,j;
	Pokemon* pokemon_aux;
	MetadataPokenest* pokenest_aux;

	for(i=0;i<list_size(entrenador_perdedor->pokemonCapturados);i++)
	{
		pokemon_aux=list_get(entrenador_perdedor->pokemonCapturados,i);
		for(j=0;j<list_size(pokenests);j++)
		{
			pokenest_aux = list_get(pokenests,j);
			if(letra_pokenest(pokemon_aux->pokemon->species)==pokenest_aux->simbolo)
				pokenest_aux->cantPokemon+=1;
		}
	}
}
*/

//*************************************************************

Jugador* batalla_deadlock(t_list* posible_deadlock,t_list* pokenests)    //BATALLAS
{
	ordenar_por_llegada(posible_deadlock);

	t_pokemon *poke2,*poke_perdedor;

	t_pkmn_factory* fabrica = create_pkmn_factory();

	int cantidad_involucrados = list_size(posible_deadlock),i=0;

	Jugador*aux1,*aux2,*entrenador_perdedor;

	Pokemon *mayor_nivel1,*mayor_nivel2;
	aux1 = (Jugador*)list_get(posible_deadlock,0);
	mayor_nivel1 = pokemon_mayor_nivel(aux1);
	poke_perdedor = create_pokemon(fabrica, mayor_nivel1->pokemon->species,mayor_nivel1->pokemon->level);
	mayor_nivel1->pokemon = poke_perdedor;

	for(i=1;i<cantidad_involucrados;i++)
	{
		aux2 = (Jugador*)list_get(posible_deadlock,i);

		mayor_nivel2 = pokemon_mayor_nivel(aux2);

		poke2 = create_pokemon(fabrica, mayor_nivel2->pokemon->species,mayor_nivel2->pokemon->level);

		mayor_nivel2->pokemon = poke2;

		poke_perdedor = pkmn_battle(poke_perdedor,poke2);
	}

	entrenador_perdedor = entrenador_tiene_pokemon(posible_deadlock,poke_perdedor);

	return entrenador_perdedor;
}
//*************************************************************

bool llego_primero(Jugador* uno,Jugador* otro)
{
	return (uno->numero)<(otro->numero);
}

//*************************************************************

void ordenar_por_llegada(t_list* entrenadores)
{
	list_sort(entrenadores, llego_primero);
}
//*************************************************************

Jugador* detectar_y_solucionar_deadlock(t_list* pokenests,t_list* entrenadores)
{
    t_list* posibles_deadlock  = list_create();

    t_list* entrenadores_aux=list_create();

    t_list* pokenests_aux=list_create();

    Jugador* perdedor =0;

    list_add_all(entrenadores_aux,entrenadores);

    list_add_all(pokenests_aux,pokenests);

	int** matriz_peticiones = generar_matriz_peticiones(entrenadores_aux, pokenests_aux);

	int** matriz_recursos_asignados = generar_matriz_asignados(entrenadores_aux, pokenests_aux);

	int* recursos_disponibles = generar_vector_recursos_disponibles(pokenests_aux);

	printf("                  PETICIONES\n");
	mostrar_matriz(matriz_peticiones,list_size(entrenadores_aux),list_size(pokenests_aux));
	printf("                  ASIGNADOS\n");
	mostrar_matriz(matriz_recursos_asignados,list_size(entrenadores_aux),list_size(pokenests_aux));

	mostrar_recursos_disponibles(recursos_disponibles, list_size(pokenests_aux));
	printf("\n\n\n");

	 if((list_size(entrenadores_aux)>1)&&(list_size(pokenests_aux)))
	 {
		posibles_deadlock = no_pueden_ejecutar(entrenadores_aux,pokenests_aux);

		printf("Cantidad de entrenadores en deadlock o inanicion: %d\n\n",list_size(posibles_deadlock));

		if(list_size(posibles_deadlock)>1)
		{
			sacar_inanicion(posibles_deadlock);
			if(list_size(posibles_deadlock)>1)
			{
				printf("Cantidad de entrenadores en algun deadlock: %d\n",list_size(posibles_deadlock));

				if(list_size(posibles_deadlock)>1)
				{
					perdedor = batalla_deadlock(posibles_deadlock,pokenests_aux);
				}
			}
			else
				printf("No hay deadlock\n\n");
		}
	 }

	printf("Muere el entrenador %d\n",perdedor);

	if(perdedor)
		printf("Y su numero es: %d\n",perdedor->numero);
		return perdedor;

		list_destroy(posibles_deadlock);

		list_destroy(entrenadores_aux);
}
//*************************************************************

t_list* obtener_un_deadlock(t_list* pokenests,t_list* entrenadores)
{
	t_list* entrenadores_aux=list_create();

	t_list* pokenests_aux=list_create();

    Jugador* perdedor =0;

    list_add_all(entrenadores_aux,entrenadores);

    list_add_all(pokenests_aux,pokenests);

    if((list_size(pokenests))&&((list_size(entrenadores))>1))
    {
		int** matriz_peticiones = generar_matriz_peticiones(entrenadores_aux, pokenests_aux);

		int** matriz_recursos_asignados = generar_matriz_asignados(entrenadores_aux, pokenests_aux);

		int* recursos_disponibles = generar_vector_recursos_disponibles(pokenests_aux);

		entrenadores_aux = no_pueden_ejecutar(entrenadores_aux,pokenests_aux);

		sacar_inanicion(entrenadores_aux);
    }

	list_destroy(pokenests_aux);

	return entrenadores_aux;
}

#endif /* HEADERS_DEADLOCK_H_ */
