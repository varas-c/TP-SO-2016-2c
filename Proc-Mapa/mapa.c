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

#define TAMANIO_BUFFER 11
t_list* colaListos;
t_list* colaBloqueados;
t_queue* colaDesconectados;

t_list* gui_items;
t_list* listaPokemon;
t_log* traceLogger;
t_log* infoLogger;

MetadataMapa mdataMapa;
MetadataPokemon mdataPokemon;
MetadataPokenest mdataPokenest;

// SEMAFOROS
pthread_mutex_t mutex_socket = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_Listos = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_Bloqueados = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_Desconectados = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_gui_items = PTHREAD_MUTEX_INITIALIZER;
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

#include "headers/planificacion.h" //Este lo puse acpa abajo porque usa variables globales, fuck it. No se como arreglarlo.


/****************************************************************************************************************
			FUNCIONES DE LECTURA DE PARAMETROS POR CONSOLA (LOS QUE SE RECIBEN POR **ARGV)
****************************************************************************************************************/

void loggearColas(void){
	t_queue *auxLista;
	auxLista = queue_create();
	Jugador* jugador;
	char* simbolos=malloc(0);
	short indice =0;
	pthread_mutex_lock(&mutex_Listos); //Mutex de listas
	pthread_mutex_lock(&mutex_Bloqueados);

	if (!queue_is_empty(colaListos)){
		jugador = (Jugador*)queue_pop(colaListos);
		while(jugador!=NULL){
			simbolos=realloc(simbolos, sizeof(simbolos)+2);
			simbolos[indice++]= jugador->entrenador.simbolo;
			simbolos[indice++]= '-';
			queue_push(auxLista, jugador);
			jugador = (Jugador*)queue_pop(colaListos);
		}

		jugador = (Jugador*)queue_pop(auxLista);
		simbolos=realloc(simbolos, sizeof(simbolos)+1);
		simbolos[indice]='\0';
		while (jugador!=NULL)
		{
			queue_push(colaListos, jugador);
			jugador = (Jugador*)queue_pop(auxLista);
		}

		log_info(infoLogger, "Jugadores en cola Listos: %s", simbolos);
		free(simbolos);
		simbolos=malloc(0);
	}
	else{
		log_info(infoLogger, "Cola Listos vacia.");
	}

	if (!queue_is_empty(colaBloqueados)){
		jugador = (Jugador*)queue_pop(colaBloqueados);
		while(jugador!=NULL){
			simbolos=realloc(simbolos, sizeof(simbolos)+2);
			simbolos[indice++]= jugador->entrenador.simbolo;
			simbolos[indice++]= '-';
			queue_push(auxLista, jugador);
			jugador = (Jugador*)queue_pop(colaBloqueados);
		}

		jugador = (Jugador*)queue_pop(auxLista);
		simbolos=realloc(simbolos, sizeof(simbolos)+1);
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
	pthread_mutex_unlock(&mutex_Listos);
	pthread_mutex_unlock(&mutex_Bloqueados);
}
//****************************************************************************************************************

void free_paquete(Paquete *paquete)
{
	free(paquete->buffer);
	paquete->tam_buffer = -1;
}
//****************************************************************************************************************

//****************************************************************************************************************


//****************************************************************************************************************

//****************************************************************************************************************


void generarColasBloqueados()
{
	colasBloqueados.cant = list_size(listaPokenest);
	colasBloqueados.cola = malloc(sizeof(t_queue*)*colasBloqueados.cant);
	colasBloqueados.referencias = malloc(sizeof(char)*colasBloqueados.cant);

	//Armo las referencias

	MetadataPokenest* pokenest;
	int i;

	for(i=0;i<colasBloqueados.cant;i++)
	{
		pokenest = list_get(listaPokenest,i);
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
		printf("Error - Func %s - Linea %d - No existe la pokenest, posicion -1",__func__,__LINE__);
		exit(1);
	}

	queue_push(colasBloqueados.cola[posicion],jugador);

}


Jugador* desbloquearJugador(char simboloPokenest)
{
	Jugador* jugadorDesbloqueado = NULL;

	int posicion = getReferenciaPokenest(simboloPokenest);

	if(posicion == -1)
	{
		printf("Error - Func %s - Linea %d - No existe la pokenest, posicion -1",__func__,__LINE__);
		exit(1);
	}

	if(queue_size(colasBloqueados.cola[posicion]) > 0)
	{
		jugadorDesbloqueado = queue_pop(colasBloqueados.cola[posicion]);
	}

	return jugadorDesbloqueado;
}



//****************************************************************************************************************

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

	//----------------Terminams! :D---------------

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

	char* pokemonDat = NULL;

	t_pkmn_factory* fabrica = create_pkmn_factory();     //SE CREA LA FABRICA PARA HACER POKEMONES

	dpPokenest = opendir(rutaPokenest); //Abrimos la rutaPokenest (PRUEBA: /mnt/pokedex/Mapas/Mapa1/Pokenest)

	Pokemon *pokemon;

	if(dpPokenest == NULL)
	{
	   printf("Error -- Funcion: %s - Linea: %d ",__func__,__LINE__);
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
		{

			//******************************
			////Encontramos una carpeta dentro de /mnt/pokedex/Mapas/Mapa1/Pokenest
			//Puede ser Pikachu,Charmande,r etc, son todas carpetas

			//Encontramos una carpeta Pokemon, hay que abrirla y leer todos sus archivos
			rutaAux = malloc(sizeof(char)*256);
			strcpy(rutaAux,rutaPokenest); //Copiamos la ruta Pokenest
			strcat(rutaAux,dptrPokenest->d_name); //Copiamos el nombre de la carpeta que hay que leer
			strcat(rutaAux,"/");

			//inicializamos las variables para una nueva Pokenest!
			pokenest = malloc(sizeof(MetadataPokenest));

			//Dentro de la carpeta hay un metadata que hay que leer!

			//Leemos la metadata
			*pokenest = leerMetadataPokenest(rutaAux,"metadata");

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
				pokemon->pokemon = create_pokemon(fabrica, dptrPokenest->d_name, mdataPokemon.nivel);
				queue_push(pokenest->colaDePokemon,pokemon);
			}

			list_add(listaPokenest,pokenest);

		}
	}

	free(rutaAux);
	free(rutaPokenest);
	closedir(dpPokenest);
	destroy_pkmn_factory(fabrica);

}

void gui_crearPokenests()
{
	int cantPokenest = list_size(listaPokenest);
	int i=0;

	MetadataPokenest* pokenest;
	for(i=0;i<cantPokenest;i++)
	{
	pokenest = list_get(listaPokenest,i);
	CrearCaja(gui_items, pokenest->simbolo, pokenest->posicionX, pokenest->posicionY,pokenest->cantPokemon);
	}


}

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

void expropiarPokemones(t_list* listaPokemones)
{
	Pokemon* pokemon = NULL;
	MetadataPokenest pokenest;;
	Jugador* jugador = NULL;

	int cantPokemones = list_size(listaPokemones);
	int i;
	int posicion;

	for(i=0;i<cantPokemones;i++)
	{
		pokemon = list_get(listaPokemones,i); //Obtengo el primer Pokemon

		jugador = desbloquearJugador(pokemon->pokenest);

		if(jugador != NULL) //Desbloqueamos un jugador de la cola de bloqueados! Hay que pasarlo a ready
		{
			pthread_mutex_lock(&mutex_Listos);
			jugador->estado = 1;
			jugador->peticionBloqueado = -1;
			list_add(jugador->pokemonCapturados,pokemon);
			list_add(colaListos,jugador);
			pthread_mutex_unlock(&mutex_Listos);
		}

		else //Ningun jugador estaba esperando este pokemon, asi que metemos al pokemon a la pokenest
		{
			pokenest = buscar_Pokenest(pokemon->pokenest);
			queue_push(pokenest.colaDePokemon,pokemon);
		}

	}

	//list_clean(listaPokemones);
}




void desconectarJugador(Jugador* jugador)
{
	close(jugador->socket);
	BorrarItem(gui_items,jugador->entrenador.simbolo);
	FD_CLR(jugador->socket,&fds_entrenadores);
	expropiarPokemones(jugador->pokemonCapturados);
	free(jugador);
}

//*************************************************************************

Paquete srlz_capturaOK(Pokemon* pokemon)
{
	Paquete paquete;
	char* pokemonDat = pokemon->nombre;
	int tamPokemonDat = strlen(pokemonDat)+1;

	paquete.tam_buffer = size_CAPTURA_OK+sizeof(char)*tamPokemonDat;
	paquete.buffer = malloc(paquete.tam_buffer);

	int codOp = CAPTURA_OK;

	memcpy(paquete.buffer,&codOp,sizeof(int));
	memcpy(paquete.buffer+sizeof(int),&tamPokemonDat,sizeof(int));
	memcpy(paquete.buffer+sizeof(int)*2,pokemonDat,sizeof(char)*tamPokemonDat);

	free(pokemonDat);
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

	jugadorBuscado= list_remove_by_condition(colaListos,_find_priority_SRDF);

	return jugadorBuscado;

}

bool any_prioritySRDF()
{

	bool flag;

	bool _any_satisfy_priority(Jugador* jugador)
	{
		return jugador->conocePokenest == FALSE;
	}


	flag = list_any_satisfy(colaListos,_any_satisfy_priority);

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

void* thread_planificador()
{
	//nivel_gui_inicializar();

	void* buffer_recv;
	int tam_buffer_recv = 100;
	int estado_socket;
	Jugador *jugador;

	int quantum = mdataMapa.quantum;
	int codOp = -1;

	char pokenestPedida;
	MetadataPokenest pokenestEnviar;
	char* mostrar = malloc(sizeof(char)*256);

	PosEntrenador pos;

	MetadataPokenest pokenest;
	bool flag_DESCONECTADO = FALSE;
	bool flag_SRDF;



	while(1)
	{
	nivel_gui_dibujar(gui_items,"                                                           ");
	sprintf(mostrar,"Mapa: %s -- No hay jugadores",parametros.nombreMapa);
	nivel_gui_dibujar(gui_items, mostrar);
	usleep(mdataMapa.retardo*1000);

	//Si nadie mas se quiere ir, es hora de Jugar!

	int retval = 0;

	while(!list_is_empty(colaListos))
	{

		if(strcmp(mdataMapa.algoritmo,"SRDF") == 0)
		{
		pthread_mutex_lock(&mutex_Listos);

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

		pthread_mutex_unlock(&mutex_Listos);
		}

		else
		{
			pthread_mutex_lock(&mutex_Listos);
			jugador = list_remove(colaListos,0);
			quantum = mdataMapa.quantum;
			flag_SRDF = FALSE;
			pthread_mutex_unlock(&mutex_Listos);
		}

		buffer_recv = malloc(tam_buffer_recv);


		//log_info(infoLogger, "Jugador %c sale de Listos.",jugador->entrenador.simbolo);
		//loggearColas();

		while(quantum > 0)
		{
			usleep(mdataMapa.retardo*1000);
			//Ya tenemos jugador, ahora le mandamos un turno
			//send_Turno(jugador->socket);
			//Ya mandamos el turno, ahora recibimos el pedido del entrenador


			estado_socket = recv(jugador->socket,buffer_recv,tam_buffer_recv,0);

			if(estado_socket <= 0)
			{
				desconectarJugador(jugador);
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
					//opc = tomarDecisionCapturaPokemon(jugador,pokenestPedida);
					pokenest = buscar_Pokenest(pokenestPedida);
					Pokemon* pokemon;

					//HASTA ACA ESTA BIEN!!!
					if(queue_size(pokenest.colaDePokemon)>0) //HAY POKEMONES PARA ENTREGAR!
					{
						pokemon = queue_pop(pokenest.colaDePokemon);
						retval = send_capturaOK(jugador,pokemon);
						flag_DESCONECTADO = verificarConexion(jugador,retval,&quantum);

						if(flag_DESCONECTADO == FALSE)
						{
							list_add(jugador->pokemonCapturados,pokemon);
							restarRecurso(gui_items,pokenest.simbolo);
							jugador->conocePokenest = false;
						}

						else
						{
							queue_push(pokenest.colaDePokemon,pokemon);
						}

						quantum=0;
					}

					else
					{
						// NO HAY MAS POKEMONS! hay que bloquear al entrenador
						jugador->estado = 1;
						jugador->peticionBloqueado = pokenestPedida;
						bloquearJugador(jugador,pokenestPedida);
						quantum=0;
					}

				break;
				case 0:
					desconectarJugador(jugador);
					quantum = 0;
					break;

				} //FIN SWITCH

			}//FIN ELSE

			calcular_coordenadas(&(jugador->entrenador),pokenestEnviar.posicionX,pokenestEnviar.posicionY);

			int tam = list_size(colaListos);

			sprintf(mostrar,"Mapa: %s - Quantum: %i - Jugador: %i - TamLista: %i",parametros.nombreMapa,quantum,jugador->numero,tam);

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
			pthread_mutex_lock(&mutex_Listos);
			list_add(colaListos,(void*)jugador);
			pthread_mutex_unlock(&mutex_Listos);
			quantum = 0;
		}
		}

	//	jugador=NULL;


		flag_DESCONECTADO = FALSE;


	}

	}
}



int main(int argc, char** argv)
{


	//verificarParametros(argc); //Verificamos que la cantidad de Parametros sea correcta
	//parametros = leerParametrosConsola(argv); //Leemos parametros por Consola

	parametros.dirPokedex = "/mnt/pokedex";
	parametros.nombreMapa = "PuebloPaleta";


	traceLogger = log_create("Logs.log", "Mapa", false, LOG_LEVEL_TRACE);
	infoLogger = log_create("Logs.log", "Mapa", false, LOG_LEVEL_INFO);
	log_info(infoLogger, "Se inicia Mapa.");


	gui_items = list_create();
	listaPokenest= list_create();
	listaPokemon = list_create();


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

	int i, j;
	FD_ZERO(&fds_entrenadores);    // borra los conjuntos maestro y temporal
	FD_ZERO(&read_fds);

	listener = socket_startListener(mdataMapa.puerto);

	// añadir listener al conjunto maestro
	FD_SET(listener, &fds_entrenadores);

	// seguir la pista del descriptor de fichero mayor
	fdmax = listener; // por ahora es éste

	// bucle principal

	colaListos = list_create();
	colaBloqueados = list_create();
	colaDesconectados = queue_create();

	//FALTAN CARGAR LAS POKENEST Y DIBUJARLAS


	pthread_t hiloPlanificador;
	int valorHilo = -1;

	valorHilo = pthread_create(&hiloPlanificador,NULL,thread_planificador,NULL);

	if(valorHilo != 0)
	{
		perror("Error al crear hilo Planificador");
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
					aux->peticionBloqueado = 0;

					//Creamos el personaje
					CrearPersonaje(gui_items,nuevoJugador.entrenador.simbolo,nuevoJugador.entrenador.posx, nuevoJugador.entrenador.posy);

					//Mutua exclusion con el planificador !
					pthread_mutex_lock(&mutex_Listos);
					list_add(colaListos, aux);
					pthread_mutex_unlock(&mutex_Listos);

					//Loggeamos info
					log_info(infoLogger, "Nuevo jugador: %c, socket %d", nuevoJugador.entrenador.simbolo, nuevoJugador.socket);
					log_info(infoLogger, "Jugador %c entra en Cola Listos", nuevoJugador.entrenador.simbolo);
					//loggearColas();
				}

			}
		}


		sem_post(&semaforo_SincroSelect);


	}

	//TODO IMPORTANTE: anticipar la cancelacion del programa y ejecutar esto.

	free(mdataPokenest.tipoPokemon);
	free(mdataMapa.algoritmo);
	free(mdataMapa.ip);
	free(mdataMapa.puerto);

	log_info(infoLogger, "Se cierra Mapa.");
	log_destroy(traceLogger);
	log_destroy(infoLogger);

	nivel_gui_terminar();
	return 0;
}
