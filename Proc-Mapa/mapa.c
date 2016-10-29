/*
 * mapa.c
 *
 *  Created on: 30/8/2016
 *      Author: utnso
 */
#include <pkmn/factory.h>
#include <pthread.h>
#include <sys/select.h>
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
#include <nivel.h>
#include <commons/log.h>
#include <commons/collections/queue.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <dirent.h>
#include <pkmn/battle.h>
#include <semaphore.h>
#include <signal.h>
/****************************************************************************************************************
			INCLUDES PROPIOS :)
****************************************************************************************************************/
#include "headers/struct.h"
#include "headers/socket.h"
#include "headers/configMapa.h"
#include "headers/serializeMapa.h"
#include "headers/pokenest.h"
#include "headers/entrenador.h"
/****************************************************************************************************************
			VARIABLES GLOBALES
****************************************************************************************************************/
t_list* listaListos;
t_list* colaBloqueados;
t_list* listaDeadlock;
t_list* gui_items;
t_list* listaPokemon;
t_log* traceLogger;
t_log* infoLogger;
t_log* deadlockLogger;

MetadataMapa mdataMapa;
MetadataPokemon mdataPokemon;
MetadataPokenest mdataPokenest;

// SEMAFOROS
pthread_mutex_t mutex_socket = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_Listos = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_Bloqueados = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_Desconectados = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_gui_items = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_hiloDeadlock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_BatallaPokemon = PTHREAD_MUTEX_INITIALIZER;

sem_t semaforo_SincroSelect;

typedef struct
{
	t_queue** cola;
	char* referencias;
	int cant;
}structColasBloqueados;

int global_cantJugadores = -1;
fd_set fds_entrenadores;
ParametrosMapa parametros;
structColasBloqueados colasBloqueados;

t_list* global_listaJugadoresSistema;

#include "headers/planificacion.h" //Este lo puse acpa abajo porque usa variables globales, fuck it. No se como arreglarlo.
#include "headers/deadlock.h"

/****************************************************************************************************************
			FUNCIONES DE LECTURA DE PARAMETROS POR CONSOLA (LOS QUE SE RECIBEN POR **ARGV)
****************************************************************************************************************/

void loggearListaListos()
{
		pthread_mutex_lock(&mutex_Listos); //Mutex de listas
		int tamLista = list_size(listaListos);
		char* simbolos = malloc(sizeof(char)*tamLista*2+1);
		strcpy(simbolos," ");
		Jugador* jugador;
		int i;

		if(tamLista > 0)
		{
			char* aux = malloc(sizeof(char)*2+1); //Pedimos memoria para un string auxiliar que nos sirve para copiar el simbolo
			aux[1] = '\0';

			for(i=0;i<tamLista;i++) //Recorremos la lista buscando todos los entrenadores y copiamos su simbolo
			{
				jugador = list_get(listaListos,i);
				aux[0] = jugador->entrenador.simbolo;
				strcat(simbolos,aux);
			}
			log_info(infoLogger, "Jugadores en cola Listos en el mapa %s: %s",parametros.nombreMapa, simbolos);
			free(aux);
		}
		else
		{
			log_info(infoLogger, "Cola Listos vacia en el mapa %s.", parametros.nombreMapa);
		}
		pthread_mutex_unlock(&mutex_Listos);
}

void loggearColaBloqueados()
{
	pthread_mutex_lock(&mutex_Bloqueados);
	int i,k;
	int cantEntrenadores = 0;

	t_queue* colaAux = queue_create(); //Cola auxiliar

	for(i=0;i<colasBloqueados.cant;i++)
	{
		cantEntrenadores += queue_size(colasBloqueados.cola[i]); //Contamos cuantos entrenadores hay en todas las listas
	}

	if(cantEntrenadores > 0) //si hay entrenadores, los informamos
	{
	char* simbolos = malloc(sizeof(char)*cantEntrenadores*2+1); //reservamos memoria para los simbolos
	char* aux = malloc(sizeof(char)*2); //auxiliar para convertir el simbolo en un string
	aux[1] = '\0';
	strcpy(simbolos," "); //inicializamos el string de simbolos

	int tam = 0;
	int j=0;
	Jugador* jugador = NULL;

	for(i=0;i<colasBloqueados.cant;i++) //Recorremos todas las colas
	{
		tam = queue_size(colasBloqueados.cola[i]); //me fijo cuantos entrenadores tiene esa cola

		for(j=0;j<tam;j++)
		{
			jugador = queue_pop(colasBloqueados.cola[i]); //saco al jugador
			aux[0] = jugador->entrenador.simbolo; //copio su simbolo
			strcat(simbolos,aux);//lo agrego al string auxiliar
			queue_push(colaAux,jugador);//guardamos al jugador en una cola auxiliar
		}

		while(queue_size(colaAux) > 0) //Devolvemos a todos los jugadores de la colaAux a la cola en la que estaban
		{
			queue_push(colasBloqueados.cola[i],queue_pop(colaAux));
		}
	}
	log_info(infoLogger, "Jugadores en cola Bloqueados en el mapa %s: %s ",parametros.nombreMapa,simbolos);
	}
	else
	{
		log_info(infoLogger, "No hay jugadores en colas de bloqueados del mapa %s", parametros.nombreMapa);
	}
	pthread_mutex_unlock(&mutex_Bloqueados);
}

void loggearColas(void)
{
	loggearListaListos();
	loggearColaBloqueados();







}

/*
void loggearColas(void){
	t_queue *auxLista;
	auxLista = queue_create();
	Jugador* jugador;
	char* simbolos=malloc(0);
	short indice =0;
	int cantListos=0;
	int cantBloqueados=0;

	pthread_mutex_lock(&mutex_Listos); //Mutex de listas

	if (!list_is_empty(listaListos))
	{
		jugador = (Jugador*)list_get(listaListos,0);
		while(jugador!=NULL)
		{
			cantListos = cantListos + 2;
			simbolos=realloc(simbolos,cantListos);
			simbolos[indice++]= jugador->entrenador.simbolo;
			simbolos[indice++]= '-';
			list_add(auxLista, jugador);
			jugador = (Jugador*)list_get(listaListos,0);
		}

		jugador = (Jugador*)list_get(auxLista,0);
		simbolos=realloc(simbolos, cantListos+1);
		simbolos[indice]='\0';
		while (jugador!=NULL)
		{
			list_add(listaListos, jugador);
			jugador = (Jugador*)list_get(auxLista,0);
		}

		log_info(infoLogger, "Jugadores en cola Listos: %s", simbolos);
		free(simbolos);
		simbolos=malloc(0);
	}
	else{
		log_info(infoLogger, "Cola Listos vacia.");
	}

	pthread_mutex_unlock(&mutex_Listos);

	pthread_mutex_lock(&mutex_Bloqueados);

	if (!queue_is_empty(colaBloqueados)){
		jugador = (Jugador*)queue_pop(colaBloqueados);
		while(jugador!=NULL){
			simbolos=realloc(simbolos, cantBloqueados+2);
			simbolos[indice++]= jugador->entrenador.simbolo;
			simbolos[indice++]= '-';
			queue_push(auxLista, jugador);
			jugador = (Jugador*)queue_pop(colaBloqueados);
		}

		jugador = (Jugador*)queue_pop(auxLista);
		simbolos=realloc(simbolos, cantBloqueados+1);
		simbolos[indice]='\0';
		while (jugador!=NULL)
		{
			queue_push(colaBloqueados, jugador);
			jugador = (Jugador*)queue_pop(auxLista);
		}

		log_info(infoLogger, "Jugadores en cola Bloqueados: %s", simbolos);
	}
	else{
		log_info(infoLogger, "Cola Bloqueados vacia.");
	}

	free(simbolos);
	queue_destroy(auxLista);

	pthread_mutex_unlock(&mutex_Bloqueados);
}*/

void free_paquete(Paquete *paquete)
{
	free(paquete->buffer);
	paquete->tam_buffer = -1;
}

void generarColasBloqueados()
{
	pthread_mutex_lock(&mutex_hiloDeadlock);
	colasBloqueados.cant = list_size(listaPokenest);
	pthread_mutex_unlock(&mutex_hiloDeadlock);
	colasBloqueados.cola = malloc(sizeof(t_queue*)*colasBloqueados.cant);
	colasBloqueados.referencias = malloc(sizeof(char)*colasBloqueados.cant);

	//Armo las referencias

	MetadataPokenest* pokenest;
	int i;

	for(i=0;i<colasBloqueados.cant;i++)
	{
		pthread_mutex_lock(&mutex_hiloDeadlock);
		pokenest = list_get(listaPokenest,i);
		pthread_mutex_unlock(&mutex_hiloDeadlock);
		colasBloqueados.referencias[i] = pokenest->simbolo;
	}
	//Creo todas las colas necesarias

	for(i=0;i<colasBloqueados.cant;i++)
	{
		colasBloqueados.cola[i] = queue_create();
	}
}

int getReferenciaPokenest(char simboloPokenest)
{
	int i;

	for(i=0;i<colasBloqueados.cant;i++)
	{
		if(colasBloqueados.referencias[i] == simboloPokenest)
		{
			return i;
		}
	}
	return -1;
}

void bloquearJugador(Jugador* jugador,char simboloPokenest)
{
	int posicion = getReferenciaPokenest(simboloPokenest);

	if(posicion == -1)
	{
		printf("Error - Func %s - Linea %d - No existe la pokenest, posicion -1\n\n",__func__,__LINE__);
		exit(1);
	}
	log_info(infoLogger, "Jugador %c entra a Bloqueados en el mapa %s.",jugador->entrenador.simbolo, parametros.nombreMapa);
	queue_push(colasBloqueados.cola[posicion],jugador);
	loggearColas();
}

Jugador* desbloquearJugador(char simboloPokenest)
{
	Jugador* jugadorDesbloqueado = NULL;

	int posicion = getReferenciaPokenest(simboloPokenest);

	if(posicion == -1)
	{
		printf("Error - Func %s - Linea %d - No existe la pokenest, posicion -1 \n\n",__func__,__LINE__);
		exit(1);
	}

	if(queue_size(colasBloqueados.cola[posicion]) > 0)
	{
		jugadorDesbloqueado = queue_pop(colasBloqueados.cola[posicion]);

		log_info(infoLogger, "Jugador %c sale de Bloqueados.",jugadorDesbloqueado->entrenador.simbolo);
		loggearColas();
	}
	return jugadorDesbloqueado;
}

//FUNCION PARA USAR CON FILE SYSTEM LOCAL!!!
int cantPokemonEnDir(char* ruta)
{
	struct dirent *archivo = NULL;
	int cantPokes = 0;
	DIR* rutaLeer = NULL;

	rutaLeer = opendir(ruta);

	if(rutaLeer == NULL)
	{
		perror("cantPokemonEnDir - open(ruta) == NULL");
		exit(1);
	}

	while(NULL != (archivo = readdir(rutaLeer)))
	{
		if(archivo->d_type == DT_REG && strcmp(archivo->d_name,"metadata") != 0) //Si es un archivo y no es metadata -> es un pokemon!
		{
			cantPokes++;
		}
	}
	closedir(rutaLeer);

	return cantPokes;
}

// FUNCION PARA USAR CON FUSE!!!!!!!!
/*int cantPokemonEnDir(char* ruta)
{
	struct dirent *archivo = NULL;
	int cantPokes = 0;
	DIR* rutaLeer = NULL;
	char* aux;

	rutaLeer = opendir(ruta);
	struct stat st;

	if(rutaLeer == NULL)
	{
		perror("cantPokemonEnDir - open(ruta) == NULL\n\n");
		exit(1);
	}

	while(NULL != (archivo = readdir(rutaLeer)))
	{
		aux = malloc(sizeof(char)*256);
		strcpy(aux, ruta);
		stat(strcat(aux,archivo->d_name), &st);

		if(st.st_mode == (S_IFREG | 0777) && strcmp(archivo->d_name,"metadata") != 0) //Si es un archivo y no es metadata -> es un pokemon!
		{
			cantPokes++;
		}
		free(aux);
	}
	closedir(rutaLeer);

	return cantPokes;
}*/

char* stringPokemonDat(char* nombrePoke, int numPoke)
{
	char *pokeDat;
	int long_poke = strlen(nombrePoke);
	int cantNumeros = 3;
	char* extension = ".dat";
	int cantExtension = strlen(extension);
	int cantBytes = long_poke+cantNumeros+cantExtension+1;
	pokeDat = malloc(sizeof(char)*cantBytes);
	snprintf(pokeDat, cantBytes, "%s%03d%s", nombrePoke, numPoke,extension);

	return pokeDat;
}

char* getRutaPokenest(ParametrosMapa parametros)
{
	char* rutaPokenest = malloc(sizeof(char)*256);
	char* Mapas = "/Mapas/";
	char* Pokenest = "/Pokenest/";

	//Hay que entrar a la ruta de la Pokedex que nos pasaron por parametro (mnt/pokedex)
	strcpy(rutaPokenest,parametros.dirPokedex);

	//Ahora a "/Mapas/"
	strcat(rutaPokenest,Mapas); // -- mnt/pokedex/Mapas/

	//Ahora a la carpeta del mapa correspondiente.
	strcat(rutaPokenest,parametros.nombreMapa); // -- mnt/pokedex/Mapas/--nombreMapa--

	//A la carpeta Pokenest
	strcat(rutaPokenest,Pokenest); // mnt/pokedex/Mapas/--nombreMapa--/Pokenest/

	//----------------Terminamos! :D---------------

	return rutaPokenest;
}

void leerTodasLasPokenest(ParametrosMapa parametros)
{
	char* rutaPokenest;
	rutaPokenest = getRutaPokenest(parametros);

	MetadataPokenest *pokenest;

	char* rutaAux = NULL;

	DIR *dpPokenest = NULL;
	struct dirent *dptrPokenest = NULL;

	t_pkmn_factory* fabrica = create_pkmn_factory();     //SE CREA LA FABRICA PARA HACER POKEMONES

	dpPokenest = opendir(rutaPokenest); //Abrimos la rutaPokenest (PRUEBA: /mnt/pokedex/Mapas/Mapa1/Pokenest)

	Pokemon *pokemon;

	if(dpPokenest == NULL)
	{
	   printf("Error -- Funcion: %s - Linea: %d \n\n",__func__,__LINE__);
	   exit(1);
	}
	//Leemos una a una las carpetas de /mnt/pokedex/Mapas/Mapa1/Pokenest
	//Carpetas: Charmander, PIkachu, etc
	while(NULL != (dptrPokenest = readdir(dpPokenest)) )
	{
		if(strcmp(dptrPokenest->d_name,".") == 0 || strcmp(dptrPokenest->d_name,"..") == 0)
		{
			continue;
		}
		else
		{	//******************************
			////Encontramos una carpeta dentro de /mnt/pokedex/Mapas/Mapa1/Pokenest
			//Puede ser Pikachu,Charmande,r etc, son todas carpetas
			//Encontramos una carpeta Pokemon, hay que abrirla y leer todos sus archivos
			rutaAux = malloc(sizeof(char)*256);
			strcpy(rutaAux,rutaPokenest); //Copiamos la ruta Pokenest
			strcat(rutaAux,dptrPokenest->d_name); //Copiamos el nombre de la carpeta que hay que leer
			strcat(rutaAux,"/");

			//inicializamos las variables para una nueva Pokenest!
			//Dentro de la carpeta hay un metadata que hay que leer!
			//Leemos la metadata
			pokenest = leerMetadataPokenest(rutaAux,"metadata");
			pokenest->colaDePokemon = queue_create();
			//Ya leimos la Pokenest, ahora hay que leer los archivos pokemon que tienen el nombre DE LA CARPETA solo que son archivos y tienen numero y .dat!

			pokenest->cantPokemon = cantPokemonEnDir(rutaAux); //Aca tenemos la cantidad maxima de PokemonXXX.dat que hay que leer
			int i;

			for(i=1;i<=pokenest->cantPokemon;i++)
			{
				pokemon = malloc(sizeof(Pokemon));
				pokemon->numero = i;
				pokemon->nombre = stringPokemonDat(dptrPokenest->d_name,i);
				pokemon->pokenest = pokenest->simbolo;
				mdataPokemon = leerMetadataPokemon(rutaAux,pokemon->nombre);
				//pokemon->pokemon = malloc(sizeof(t_pokemon*));
				pokemon->pokemon = create_pokemon(fabrica, dptrPokenest->d_name, mdataPokemon.nivel);
				pokemon->pokemon->species = strdup(dptrPokenest->d_name);
				queue_push(pokenest->colaDePokemon,pokemon);
			}

			pthread_mutex_lock(&mutex_hiloDeadlock);
			list_add(listaPokenest,pokenest);
			pthread_mutex_unlock(&mutex_hiloDeadlock);
		}
	}

	free(rutaAux);
	free(rutaPokenest);
	closedir(dpPokenest);
	destroy_pkmn_factory(fabrica);
}

void gui_crearPokenests()
{
	pthread_mutex_lock(&mutex_hiloDeadlock);
	int cantPokenest = list_size(listaPokenest);
	pthread_mutex_unlock(&mutex_hiloDeadlock);

	int i=0;

	MetadataPokenest* pokenest;
	for(i=0;i<cantPokenest;i++)
	{
		pthread_mutex_lock(&mutex_hiloDeadlock);
		pokenest = list_get(listaPokenest,i);
		pthread_mutex_unlock(&mutex_hiloDeadlock);
		CrearCaja(gui_items, pokenest->simbolo, pokenest->posicionX, pokenest->posicionY,pokenest->cantPokemon);
	}
}

/*
void gui_liberarPokemons(Jugador *jugador)
{
	int cantPokenest = list_size(listaPokenest);
	int cantPokemones = list_size(jugador->pokemonCapturados);
	int i=0;
	int j=0;

	MetadataPokenest* pokenest;

	for(j=0;j<cantPokemones;j++)
	{
		for(i=0;i<cantPokenest;i++)
		{ 	pokenest = list_get(listaPokenest,i);
			if(jugador->conocePokenest)
			{
				sumarRecurso(gui_items,pokenest->simbolo);
			}
		}
	}
}

*/

/*
void printfLista()
{
	int cant;
	cant = list_size(listaPokenest);
	Pokemon* pokemon;
	MetadataPokenest *pokenest;
	int i=0;

	for(i=0;i<cant;i++)
	{
		pokenest = list_get(listaPokenest,i);

		printf("Simbolo: %c \n",pokenest->simbolo);

		while(queue_size(pokenest->colaDePokemon) != 0)
		{
			pokemon = queue_pop(pokenest->colaDePokemon);
			printf("Pokemon: %s Numero %i \n",pokemon->pokemon->species,pokemon->numero);
		}
		printf("\n \n");
	}
}
*/

typedef struct
{
	Pokemon* pokemon;
	Jugador* jugador;
}JugadorBloqueado;

t_list* expropiarPokemones(t_list* listaPokemones)
{
	JugadorBloqueado* jugadorBloqueado;
	t_list* lista_jugadoresBloqueados = list_create();

	Pokemon* pokemonDesbloqueado = NULL;
	MetadataPokenest* pokenest;
	Jugador* jugadorDesbloqueado = NULL;

	int cantPokemones = list_size(listaPokemones);
	int i;

	for(i=0;i<cantPokemones;i++)
	{
		pokemonDesbloqueado = list_get(listaPokemones,i); //Obtengo el primer Pokemon

		jugadorDesbloqueado = desbloquearJugador(pokemonDesbloqueado->pokenest);

		if(jugadorDesbloqueado != NULL) //Desbloqueamos un jugador, agrego al jugador y a su pokemon,
		{								//	a la lista para desbloquear
			jugadorBloqueado = malloc(sizeof(JugadorBloqueado));
			jugadorBloqueado->jugador = jugadorDesbloqueado; //Pasamos el jugador al struct
			jugadorBloqueado->pokemon = pokemonDesbloqueado;
			list_add(lista_jugadoresBloqueados,jugadorBloqueado);
		}

		else //Ningun jugador estaba esperando este pokemon, asi que metemos al pokemon a la pokenest
		{
			pokenest = buscar_Pokenest(pokemonDesbloqueado->pokenest);
			queue_push(pokenest->colaDePokemon,pokemonDesbloqueado);
			sumarRecurso(gui_items,pokenest->simbolo);
		}
	}

	return lista_jugadoresBloqueados;
}

void desconectarJugador(Jugador* jugador)
{
	int socket = jugador->socket;
	close(socket);
	BorrarItem(gui_items,jugador->entrenador.simbolo);
	list_clean(jugador->pokemonCapturados);
	list_destroy(jugador->pokemonCapturados);
	//free(jugador->entrenador);
	free(jugador);

	FD_CLR(socket,&fds_entrenadores);
}

Paquete srlz_capturaOK(Pokemon* pokemon)
{
	Paquete paquete;
	char* pokemonDat = pokemon->nombre;
	int tamPokemonDat = strlen(pokemonDat)+1;

	int tamSpecies = strlen(pokemon->pokemon->species)+1;

	paquete.tam_buffer = size_CAPTURA_OK+sizeof(char)*tamPokemonDat+sizeof(int)+sizeof(char)*tamSpecies;
	paquete.buffer = malloc(paquete.tam_buffer);

	int codOp = CAPTURA_OK;

	memcpy(paquete.buffer,&codOp,sizeof(int));
	memcpy(paquete.buffer+sizeof(int),&tamPokemonDat,sizeof(int));
	memcpy(paquete.buffer+sizeof(int)*2,pokemonDat,sizeof(char)*tamPokemonDat);
	memcpy(paquete.buffer+sizeof(int)*2+sizeof(char)*tamPokemonDat,&tamSpecies,sizeof(int));
	memcpy(paquete.buffer+sizeof(int)*2+sizeof(char)*tamPokemonDat+sizeof(int),pokemon->pokemon->species,sizeof(char)*tamSpecies);

	memcpy(paquete.buffer+sizeof(int)*2+sizeof(char)*tamPokemonDat+sizeof(int)+sizeof(char)*tamSpecies,pokemon->pokemon,sizeof(t_pokemon));

	//free(pokemonDat);
	return paquete;
}

int send_capturaOK(Jugador* jugador,Pokemon* pokemon)
{
	Paquete paquete;

	paquete = srlz_capturaOK(pokemon);
	int retval;

	retval = send(jugador->socket,paquete.buffer,paquete.tam_buffer,0);

	free(paquete.buffer);

	return retval;
}

Paquete srlz_MoverOK()
{
	Paquete paquete;
	paquete.tam_buffer = size_MOVER_OK;
	paquete.buffer = malloc(paquete.tam_buffer);

	int codOp = MOVER_OK;

	memcpy(paquete.buffer,&codOp,sizeof(int));

	return paquete;
}

int send_MoverOK(int socket)
{
	Paquete paquete;
	paquete = srlz_MoverOK();
	int retval;
	retval = send(socket,paquete.buffer,paquete.tam_buffer,0);

	free(paquete.buffer);

	return retval;
}

int verificarConexion(Jugador* jugador,int retval,int* quantum)
{
	if(retval < 0)
	{
		desconectarJugador(jugador);
		*quantum = 0;
		return 1;
	}

	return 0;
}

void sigHandler_reloadMetadata(int signal)
{
	if(signal == SIGUSR2)
	{
		mdataMapa = leerMetadataMapa(parametros);
	}
}

Jugador* get_prioritySRDF()
{
	Jugador* jugadorBuscado;

	bool _find_priority_SRDF(Jugador* jugador)
	{
		return jugador->conocePokenest == FALSE;
	}
	jugadorBuscado= list_remove_by_condition(listaListos,(void*)_find_priority_SRDF);

	return jugadorBuscado;
}

bool any_prioritySRDF()
{
	bool flag;
	bool _any_satisfy_priority(Jugador* jugador)
	{
		return jugador->conocePokenest == FALSE;
	}
	flag = list_any_satisfy(listaListos,(void*)_any_satisfy_priority);

	return flag;
}

void calcular_coordenadas(Entrenador* entrenador, int x, int y)
{
	//Pregunto por X
	if(x > entrenador->posx)
	{
		entrenador ->destinox = fabs(x - entrenador->posx);
	}
	if(entrenador->posx == x)
	{
		entrenador ->destinox = 0;
	}
	if(x < entrenador->posx)
	{
		entrenador->destinox = fabs(x - entrenador->posx);
	}
	//Pregunto por y
	if(y > entrenador->posy)
	{
		entrenador ->destinoy = fabs(y - entrenador->posy);
	}
	if(entrenador->posy == y)
	{
		entrenador ->destinoy = 0;
	}
	if(y < entrenador->posy)
	{
		entrenador->destinoy = fabs(y - entrenador->posy);
	}
}

void desbloquearJugadores(t_list* lista)
{
	t_list* listaAux = list_create();

	if(lista != NULL)
	{
		JugadorBloqueado* jugadorBloqueado;
		int retval;
		int i;
		int tamLista = list_size(lista);

		if(list_size(lista) > 0)
		{
			for(i=0;i<tamLista;i++)
			{
				jugadorBloqueado = list_remove(lista,0); //Hay que informarle que capturó

				retval = send_capturaOK(jugadorBloqueado->jugador,jugadorBloqueado->pokemon); //Si acá tira error, deberia sacarle lospokemon, agregar jugadoresBloqueados, desconectarlo

				if(retval<=0)
				{
					listaAux = expropiarPokemones(jugadorBloqueado->jugador->pokemonCapturados);
					borrarJugadorSistema(jugadorBloqueado->jugador);
					desconectarJugador(jugadorBloqueado->jugador);
					send_BatallaGanador(listaAux);
					list_add_all(lista,listaAux);
					tamLista = list_size(lista);
				}
				else
				{
				jugadorBloqueado->jugador->estado = 0;
				jugadorBloqueado->jugador->peticion = 0;
				list_add(jugadorBloqueado->jugador->pokemonCapturados,jugadorBloqueado->pokemon);
				list_add(listaListos,jugadorBloqueado->jugador);
				log_info(infoLogger, "Jugador %c entra a Listos en el mapa %s.",jugadorBloqueado->jugador->entrenador.simbolo, parametros.nombreMapa);
				loggearColas();
				}
				free(jugadorBloqueado);
			}
		}
	}
	list_destroy(listaAux);
}

void borrarJugadorSistema(Jugador* jugador)
{
	int socketBuscado = jugador->socket;

	bool _find_socket_(Jugador* removido)
	{
		return removido->socket == socketBuscado;
	}

	list_remove_by_condition(global_listaJugadoresSistema,(void*)_find_socket_);
}

int send_codigoOperacion(int socket,int operacion)
{
	Paquete paquete;
	paquete.buffer = malloc(sizeof(int));

	memcpy(paquete.buffer,&operacion,sizeof(int));

	int retval = send(socket,paquete.buffer,sizeof(int),0);

	free(paquete.buffer);

	return retval;
}

Paquete srlz_pedirPokemon()
{
	Paquete paquete;
	paquete.tam_buffer = size_BATALLA_PELEA;
	paquete.buffer = malloc(paquete.tam_buffer);

	int cod = BATALLA_PELEA;

	memcpy(paquete.buffer,&cod,sizeof(int));

	return paquete;
}

int send_pedirPokemonMasFuerte(Jugador* jugador)
{
	int retval;
	Paquete paquete;
	paquete = srlz_pedirPokemon();

	retval = send(jugador->socket,paquete.buffer,paquete.tam_buffer,0);
	free(paquete.buffer);

	return retval;
}

Paquete recv_pedirPokemonMasFuerte(Jugador* jugador, int* retval)
{
	Paquete paquete;
	paquete.tam_buffer = size_PEDIR_POKEMON_MAS_FUERTE;
	paquete.buffer = malloc(paquete.tam_buffer);

	*retval = recv(jugador->socket,paquete.buffer,paquete.tam_buffer,0);

	return paquete;
}

int dsrlz_pedirPokemonMasFuerte(Paquete* paquete)
{
	int indicePoke;
	int codop;

	memcpy(&codop,paquete->buffer,sizeof(int));

	if(codop != PEDIR_POKEMON_MAS_FUERTE)
	{
		printf("Error: %s - Linea %d --- codigo invalido de PEDIR_POKEMON_MAS_FUERTE", __func__,__LINE__);
		exit(1);
	}
	memcpy(&indicePoke,paquete->buffer + sizeof(int),sizeof(int));

	return indicePoke;
}

char* generarInformeBatalla(Jugador* jugador1, Jugador* jugador2,t_pokemon* pokemon1, t_pokemon* pokemon2)
{
	char* informeBatalla = malloc(sizeof(char)*256);

	sprintf(informeBatalla,"Resultado Pelea\nEntrenador1: %c - Entrenador2: %c\nPokemon1: %s - Nivel %i\nPokemon2: %s - Nivel %i\nGanador: ",
						jugador1->entrenador.simbolo,jugador2->entrenador.simbolo,pokemon1->species,pokemon1->level,pokemon2->species,pokemon2->level);

	return informeBatalla;
}

void send_informeBatalla(int socket_j1,int socket_j2,char* informeBatalla)
{
	int codop = BATALLA_INFORME;
	int tamCadena = strlen(informeBatalla)+1;
	Paquete paquete;

	paquete.tam_buffer = size_BATALLA_INFORME;
	paquete.buffer = malloc(paquete.tam_buffer);

	memcpy(paquete.buffer,&codop,sizeof(int));
	memcpy(paquete.buffer+sizeof(int),&tamCadena,sizeof(int));

	send(socket_j1,paquete.buffer,paquete.tam_buffer,0);
	send(socket_j2,paquete.buffer,paquete.tam_buffer,0);

	free(paquete.buffer);

	paquete.tam_buffer = sizeof(char)*tamCadena;
	paquete.buffer = malloc(paquete.tam_buffer);

	memcpy(paquete.buffer,informeBatalla,sizeof(char)*tamCadena);

	send(socket_j1,paquete.buffer,paquete.tam_buffer,0);
	send(socket_j2,paquete.buffer,paquete.tam_buffer,0);

	free(paquete.buffer);
}

void send_BatallaMuerte(int socket)
{
	Paquete paquete;
	paquete.tam_buffer = size_BATALLA_MUERTE;
	paquete.buffer = malloc(paquete.tam_buffer);

	int codop = BATALLA_MUERTE;

	memcpy(paquete.buffer,&codop,sizeof(int));
	send(socket,paquete.buffer,paquete.tam_buffer,0);
}

Jugador* pelearEntrenadores()
{
	int retval;
	Jugador* jugador1 = NULL;
	Jugador* jugador2 = NULL;
	Pokemon* pokemon1 = NULL;
	Pokemon* pokemon2 = NULL;
	int indicePoke;
	Paquete paquete;
	char* informeBatalla = NULL;
	Jugador* perdedor = NULL;
	char* cadenaAux = malloc(sizeof(char)*2);
	cadenaAux[1] = '\0';

	t_pokemon* pokemonPerdedor = NULL;

	jugador1 = list_remove(listaDeadlock,0);

	while(list_size(listaDeadlock) > 0)
	{
		jugador2 = list_remove(listaDeadlock,0);

		retval = send_pedirPokemonMasFuerte(jugador1);

		if(retval <= 0) //EL entrenador se fue, lo damos por muerto!
		{
			return jugador1;
		}

		retval = send_pedirPokemonMasFuerte(jugador2);

		if(retval <= 0) //EL entrenador se fue, lo damos por muerto!
		{
			return jugador2;
		}

		//Si ninguno se fue, seguimos jugando
		paquete = recv_pedirPokemonMasFuerte(jugador1, &retval);

		if(retval <= 0) //EL entrenador se fue, lo damos por muerto!
		{
			return jugador1;
		}

		indicePoke = dsrlz_pedirPokemonMasFuerte(&paquete);
		free(paquete.buffer);
		pokemon1 = list_get(jugador1->pokemonCapturados,indicePoke);

		paquete = recv_pedirPokemonMasFuerte(jugador2, &retval);

		if(retval <= 0) //EL entrenador se fue, lo damos por muerto!
		{
			return jugador2;
		}

		indicePoke = dsrlz_pedirPokemonMasFuerte(&paquete);
		free(paquete.buffer);
		pokemon2 = list_get(jugador2->pokemonCapturados,indicePoke);
		//HAY PELEA!

		pokemonPerdedor = pkmn_battle(pokemon1->pokemon,pokemon2->pokemon);

		if(pokemon1->pokemon == pokemonPerdedor) //Si el 1 perdió
		{
			informeBatalla = generarInformeBatalla(jugador1,jugador2,pokemon1->pokemon,pokemon2->pokemon);
			cadenaAux[0] = jugador2->entrenador.simbolo;
			strcat(informeBatalla,cadenaAux);
			send_informeBatalla(jugador1->socket,jugador2->socket,informeBatalla);
			free(informeBatalla);
			perdedor = jugador1;
		}
		else if(pokemon2->pokemon == pokemonPerdedor) //Si el 2 perdió
		{
			informeBatalla = generarInformeBatalla(jugador1,jugador2,pokemon1->pokemon,pokemon2->pokemon);
			cadenaAux[0] = jugador1->entrenador.simbolo;
			strcat(informeBatalla,cadenaAux);
			send_informeBatalla(jugador1->socket,jugador2->socket,informeBatalla);
			free(informeBatalla);
			perdedor = jugador2;
		}
		else
		{
			printf("Linea: %s - Linea: %d - Error De Deadlock, no se puede definir quien gano/perdio",__func__,__LINE__);
			exit(1);
		}
	}
	free(cadenaAux);
	send_BatallaMuerte(perdedor->socket);

	return perdedor;
}

void send_BatallaGanador(t_list* jugadoresBloqueados)
{
	JugadorBloqueado* jugador;
	Paquete paquete;
	paquete.tam_buffer = sizeof(int);
	paquete.buffer = malloc(paquete.tam_buffer);
	int codop = BATALLA_GANADOR;

	memcpy(paquete.buffer,&codop,sizeof(int));

	int i;
	int tamLista = list_size(jugadoresBloqueados);

	for(i=0;i<tamLista;i++)
	{
		jugador = list_get(jugadoresBloqueados,i);
		send(jugador->jugador->socket,paquete.buffer,paquete.tam_buffer,0);
	}
	free(paquete.buffer);
}

void borrarJugadorDeColaBloqueados(Jugador* jugadorBuscado)
{
	int i;
	int tam;
	int j;
	t_queue* colaAux = queue_create();
	Jugador* jugador;

	for(i=0;i<colasBloqueados.cant;i++) //Recorremos todas las colas
	{
		tam = queue_size(colasBloqueados.cola[i]); //me fijo cuantos entrenadores tiene esa cola

		for(j=0;j<tam;j++)
		{
			jugador = queue_pop(colasBloqueados.cola[i]); //saco al jugador

			if(jugadorBuscado != jugador)
			{
				queue_push(colaAux,jugador);//guardamos al jugador en una cola auxiliar
			}
		}
		while(queue_size(colaAux) > 0) //Devolvemos a todos los jugadores de la colaAux a la cola en la que estaban
		{
			queue_push(colasBloqueados.cola[i],queue_pop(colaAux));
		}
	}
}

void* thread_planificador()
{
	nivel_gui_inicializar();
	void* buffer_recv;
	int tam_buffer_recv = 100;
	int estado_socket;
	Jugador *jugador;

	int quantum = mdataMapa.quantum;
	int codOp = -1;

	char pokenestPedida;
	MetadataPokenest* pokenestEnviar;
	char* mostrar = malloc(sizeof(char)*256);

	PosEntrenador pos;
	MetadataPokenest* pokenest;
	bool flag_DESCONECTADO = FALSE;
	bool flag_SRDF;
	Pokemon* pokemon;
	t_list* lista_jugadoresBloqueados = NULL;
	pid_t pid= getpid();

	Jugador* jugadorDeadlock = NULL;

	while(1)
	{
	nivel_gui_dibujar(gui_items,"                                                           ");
	sprintf(mostrar,"Mapa: %s -- No hay jugadores -pid.%i                          ",parametros.nombreMapa,pid);
	nivel_gui_dibujar(gui_items, mostrar);
	usleep(mdataMapa.retardo*1000);

	//Si nadie mas se quiere ir, es hora de Jugar!

	int retval = 0;

	while(!list_is_empty(global_listaJugadoresSistema))
	{
		pthread_mutex_lock(&mutex_hiloDeadlock);
		if(!list_is_empty(listaDeadlock))
		{
			jugadorDeadlock = pelearEntrenadores();
			borrarJugadorDeColaBloqueados(jugadorDeadlock);
			lista_jugadoresBloqueados = expropiarPokemones(jugadorDeadlock->pokemonCapturados);

			borrarJugadorSistema(jugadorDeadlock);

			desconectarJugador(jugadorDeadlock);

			send_BatallaGanador(lista_jugadoresBloqueados);
			desbloquearJugadores(lista_jugadoresBloqueados);
			loggearColas();
		}
		pthread_mutex_unlock(&mutex_hiloDeadlock);

		if(!list_is_empty(listaListos))
		{
			if(strcmp(mdataMapa.algoritmo,"SRDF") == 0)
			{
				if(any_prioritySRDF())
				{
					jugador = get_prioritySRDF();
					quantum = 1;
					flag_SRDF = FALSE;
				}
				else{
				jugador = get_SRDF();
				flag_SRDF = TRUE;
				quantum = 1;
				}
			}
			else
			{
				jugador = list_remove(listaListos,0);
				log_info(infoLogger, "el jugador :%c ha salido de la lista de listos del mapa:%s",jugador->entrenador.simbolo, parametros.nombreMapa);
				loggearColas();
				quantum = mdataMapa.quantum;
				flag_SRDF = FALSE;
			}

			buffer_recv = malloc(tam_buffer_recv);

		while(quantum > 0)
		{
			usleep(mdataMapa.retardo*1000);
			estado_socket = recv(jugador->socket,buffer_recv,tam_buffer_recv,0);

			if(estado_socket <= 0)
			{
				quantum = 0;
				flag_DESCONECTADO = TRUE;
			}
			else
			{
				//Tomamos el primer int del buffer para ver el código de operacion
				codOp = dsrlz_codigoOperacion(buffer_recv);

				//Evaluo el codigo de Operacion para ver que verga quiere
				switch(codOp)
				{
				case POKENEST: //Nos pidieron una pokenest, hay que entregarla:
					pokenestPedida = dsrlz_Pokenest(buffer_recv); //Obtenemos el simbolo de la pokenest que nos pidieron
					pokenestEnviar = buscar_Pokenest(pokenestPedida); //Buscamos la info de la Pokenest pedida
					retval = send_Pokenest(jugador->socket,pokenestEnviar); //Enviamos el paquete :D
					jugador->conocePokenest = TRUE;
					flag_DESCONECTADO = verificarConexion(jugador,retval,&quantum);
				break;

				case MOVER: //El entrenador se quiere mover
					pos = dsrlz_movEntrenador(buffer_recv);//Obtengo las coordenadas X,Y
					movEntrenador(pos,jugador);//Actualizamos el entrenador con las nuevas coordenadas
					retval = send_MoverOK(jugador->socket);
					MoverPersonaje(gui_items, jugador->entrenador.simbolo, jugador->entrenador.posx, jugador->entrenador.posy);
					flag_DESCONECTADO = verificarConexion(jugador,retval,&quantum);
					break;

				case CAPTURAR: //TODO: FALTA COMPLETAR!!
					pokenestPedida = dsrlz_Pokenest(buffer_recv);//Identificamos la pokenest pedida
					pokenest = buscar_Pokenest(pokenestPedida);
					//HASTA ACA ESTA BIEN!!!
					//pthread_mutex_lock(&mutex_hiloDeadlock);
					if(queue_size(pokenest->colaDePokemon)>0) //HAY POKEMONES PARA ENTREGAR!
					{
						pokemon = (Pokemon*)queue_pop(pokenest->colaDePokemon);
						send_codigoOperacion(jugador->socket,CAPTURA_OK);
						retval = send_capturaOK(jugador,pokemon);
						flag_DESCONECTADO = verificarConexion(jugador,retval,&quantum);
						log_info(infoLogger,"el jugador:%c ha ingresado en la lista de listos del mapa:%s",jugador->entrenador.simbolo,parametros.nombreMapa);
						loggearColas();
						if(flag_DESCONECTADO == FALSE)
						{
							pokenest->cantPokemon--;
							list_add(jugador->pokemonCapturados,pokemon);
							restarRecurso(gui_items,pokenest->simbolo);
							jugador->conocePokenest = false;
						}
						else
						{
							queue_push(pokenest->colaDePokemon,pokemon);
						}
						quantum=0;
					}
					else
					{	// NO HAY MAS POKEMONS! hay que bloquear al entrenador
						retval = send_codigoOperacion(jugador->socket,CAPTURA_BLOQUEADO);
						flag_DESCONECTADO = verificarConexion(jugador,retval,&quantum);

						if(flag_DESCONECTADO == FALSE)
						{
							jugador->estado = 1;
							jugador->peticion = pokenestPedida;
							jugador->conocePokenest = false;
							bloquearJugador(jugador,pokenestPedida);
							log_info(infoLogger,"El Jugador %c ha entrado a la cola de Bloqueados del mapa:%s",jugador->entrenador.simbolo,parametros.nombreMapa);
							loggearColas();
						}
						quantum=0;
					}
					//pthread_mutex_unlock(&mutex_hiloDeadlock);

				break;

				case 0:
					//sumarRecurso(jugador->pokemonCapturados,pokenest->simbolo);
					lista_jugadoresBloqueados = expropiarPokemones(jugador->pokemonCapturados);
					desconectarJugador(jugador);
					log_info(infoLogger, "El jugador %c ha salido del mapa%s\n\n",jugador->entrenador.simbolo,parametros.nombreMapa);
					loggearColas();
					quantum = 0;
					flag_DESCONECTADO = TRUE;
					break;
				} //FIN SWITCH
			}//FIN ELSE

			calcular_coordenadas(&(jugador->entrenador),pokenestEnviar->posicionX,pokenestEnviar->posicionY);

			int tam = list_size(listaListos);

			sprintf(mostrar,"Mapa: %s -pid.%i - Quantum: %i - Jugador: %i - TamLista: %i ",parametros.nombreMapa,pid,quantum,jugador->numero,tam);

			//free(buffer_recv);
			//log_trace(traceLogger, "Termina turno de jugador %c", jugador->entrenador.simbolo);
			//log_info(infoLogger, "Jugador %c entra en Cola Listos", jugador->entrenador.simbolo);
			//loggearColas();
			//pthread_mutex_unlock(&mutex_socket);
			//log_info(infoLogger, "%s ha ingresado a la cola de listos con ip %d y el socket %d.",(&jugador)-> entrenador, (&jugador)-> estado, (&jugador)->socket);
			//loggearColas();
			//pthread_mutex_lock(&mutex_socket);
			if(flag_SRDF == FALSE)
			{
			quantum--;
			}
			nivel_gui_dibujar(gui_items, mostrar);
		}//FIN WHILE
		if(flag_DESCONECTADO == FALSE)
		{
		if(jugador->estado==0)
		{
			pthread_mutex_lock(&mutex_hiloDeadlock);
			list_add(listaListos,(void*)jugador);
			pthread_mutex_unlock(&mutex_hiloDeadlock);
			quantum = 0;
		}
		}
		if(flag_DESCONECTADO == TRUE)
		{
		pthread_mutex_lock(&mutex_hiloDeadlock);
		lista_jugadoresBloqueados = expropiarPokemones(jugador->pokemonCapturados);

		borrarJugadorSistema(jugador);
		desconectarJugador(jugador);
		quantum = 0;
		flag_DESCONECTADO = TRUE;
		send_BatallaGanador(lista_jugadoresBloqueados);
		desbloquearJugadores(lista_jugadoresBloqueados);
		pthread_mutex_unlock(&mutex_hiloDeadlock);
		}
		flag_DESCONECTADO = FALSE;
	}
	} //While global
	}
}

void* thread_deadlock()
{
	t_list * entrenadores_aux;

	while(1)
	{
		usleep(mdataMapa.tiempoChequeoDeadlock*1000); //EXAGERO PARA PROBAR

		pthread_mutex_lock(&mutex_hiloDeadlock);

		if(list_size(global_listaJugadoresSistema) > 0)
		{
			entrenadores_aux = obtener_un_deadlock(listaPokenest,global_listaJugadoresSistema,infoLogger);

			if(!list_is_empty(entrenadores_aux))
			{
				list_add_all(listaDeadlock,entrenadores_aux);
			}

			list_clean(entrenadores_aux);
		}
		else
			log_info(deadlockLogger, "    NO HAY ENTRENADORES EN EL MAPA % s",parametros.nombreMapa);

		pthread_mutex_unlock(&mutex_hiloDeadlock);
	}
}

int main(int argc, char** argv)
{
	verificarParametros(argc); //Verificamos que la cantidad de Parametros sea correcta
	parametros = leerParametrosConsola(argv); //Leemos parametros por Consola

	//parametros.dirPokedex = "/home/utnso/tp-2016-2c-Breaking-Bug/mnt/pokedex/";
	//parametros.nombreMapa = "PuebloPaleta";

	listaDeadlock = list_create();

	signal(SIGUSR2, sigHandler_reloadMetadata);

	traceLogger = log_create("Logs.log", "Mapa", false, LOG_LEVEL_TRACE);
	infoLogger = log_create("LogMapa.log", "Mapa", false, LOG_LEVEL_INFO);
	deadlockLogger = log_create("LogDeadlock.log", "Mapa", false, LOG_LEVEL_INFO);

	log_info(infoLogger, "Se inicia Mapa: %s.", parametros.nombreMapa);

	gui_items = list_create();
	listaPokenest= list_create();
	listaPokemon = list_create();
	global_listaJugadoresSistema = list_create();

	leerTodasLasPokenest(parametros);
	gui_crearPokenests();

	mdataMapa = leerMetadataMapa(parametros);

	generarColasBloqueados();
	//**********************************
	//Para crear una entrada en un archivo LOG:
	//log_tipoDeLog (logger, "mensaje"). tipoDeLog = trace, info, error, etc
	//*********************************
	//FUNCION SERVE

	fd_set read_fds; // conjunto temporal de descriptores de fichero para select()
	// conjunto maestro de descriptores de fichero

	int fdmax;        // número máximo de descriptores de fichero
	int listener;     // descriptor de socket a la escucha

	int i;
	FD_ZERO(&fds_entrenadores);    // borra los conjuntos maestro y temporal
	FD_ZERO(&read_fds);

	listener = socket_startListener(mdataMapa.puerto);

	// añadir listener al conjunto maestro
	FD_SET(listener, &fds_entrenadores);

	// seguir la pista del descriptor de fichero mayor
	fdmax = listener; // por ahora es éste

	// bucle principal

	listaListos = list_create();
	colaBloqueados = list_create();

	//FALTAN CARGAR LAS POKENEST Y DIBUJARLAS
	pthread_t hiloPlanificador;
	int valorHilo = -1;

	valorHilo = pthread_create(&hiloPlanificador,NULL,thread_planificador,NULL);

	if(valorHilo != 0)
	{
		perror("Error al crear hilo Planificador\n\n");
		exit(1);
	}

	pthread_t hiloDeadlock;

	valorHilo = pthread_create(&hiloDeadlock,NULL,thread_deadlock,NULL);

	if(valorHilo != 0)
	{
		perror("Error al crear hilo Deadlock");
		exit(1);
	}

	Jugador nuevoJugador;
	Jugador *aux;
	int *aux2;
	int tambuffer;
	tambuffer = sizeof(int);
	void* buffer;
	int newfd;
	char simboloEntrenador;

	for (;;) {
		read_fds = fds_entrenadores; // cópialo

		//Buscamos los sockets que quieren realizar algo con Select
		socket_select(fdmax, &read_fds);

		//Recorremos los sockets con pedidos
		for(i = 0; i <= fdmax; i++) {

			if (FD_ISSET(i, &read_fds)) {

				//Si es el Listener, tenemos NUEVA CONEXION!
				if (i == listener) {

					//SE ACEPTA UN NUEVO ENTRENADOR
					newfd = socket_addNewConection(listener,&fds_entrenadores,&fdmax);
					simboloEntrenador = recv_simboloEntrenador(newfd);
					global_cantJugadores++;
					nuevoJugador = new_Jugador(simboloEntrenador,newfd, global_cantJugadores);

					aux = malloc(sizeof(Jugador)); //NO HACERLE FREE !!!!!!!!! LIBERAR CON EL PUNTERO PUSHEADO A LA COLA DE LISTOS
					aux->entrenador = nuevoJugador.entrenador;
					aux->socket = nuevoJugador.socket;
					aux->estado = nuevoJugador.estado;
					aux->pokemonCapturados = list_create();
					aux->numero = global_cantJugadores;
					aux->conocePokenest = FALSE;
					aux->peticion = 0;

					//Creamos el personaje
					CrearPersonaje(gui_items,nuevoJugador.entrenador.simbolo,nuevoJugador.entrenador.posx, nuevoJugador.entrenador.posy);

					//Mutua exclusion con el planificador !

					pthread_mutex_lock(&mutex_hiloDeadlock);
					list_add(global_listaJugadoresSistema,aux);
					list_add(listaListos, aux);
					pthread_mutex_unlock(&mutex_hiloDeadlock);

					//Loggeamos info
					log_info(infoLogger, "Nuevo jugador: %c, socket %d en el mapa %s.", nuevoJugador.entrenador.simbolo, nuevoJugador.socket,parametros.nombreMapa);
					log_info(infoLogger, "Jugador %c entra en Lista Listos en el Mapa:%s", nuevoJugador.entrenador.simbolo,parametros.nombreMapa);
					loggearColas();
				}
			}
		}
	}

	//TODO IMPORTANTE: anticipar la cancelacion del programa y ejecutar esto.

	free(mdataPokenest.tipoPokemon);
	free(mdataMapa.algoritmo);
	free(mdataMapa.ip);

	nivel_gui_terminar();

	log_info(infoLogger, "Se cierra Mapa: %s.",  parametros.nombreMapa);
	log_destroy(traceLogger);
	log_destroy(infoLogger);
	log_destroy(deadlockLogger);

	return 0;
}
