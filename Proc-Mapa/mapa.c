/*
 * mapa.c
 *
 *  Created on: 30/8/2016
 *      Author: utnso
 */
#include <pthread.h>
#include <stdio.h>
#include <tad_items.h>
#include <string.h>
#include <stdlib.h>
#include <commons/config.h>
#include <sys/ioctl.h>
#include <curses.h>
#include "headers/struct.h"
#include <nivel.h>
#include <commons/collections/list.h>
#include "headers/socket.h"
#include <commons/log.h>
#include <commons/collections/queue.h>
#include <math.h>
#include <ctype.h>


//VARIABLES GLOBALES
#define TAMANIO_BUFFER 11
t_queue* colaListos;
t_queue* colaBloqueados;
t_list* listaDibujo;
t_log* traceLogger;
t_log* infoLogger;


typedef struct{
	char simbolo;
	int posx;
	int posy;
	char movAnterior;
	int flagx;
	int flagy;
	char* pokemones;
}Entrenador;

typedef struct
{
	Entrenador entrenador;
	int estado;
	int socket;
}Jugador;

struct nodo
{
	Jugador jugador;
	struct nodo* sgte;
};
typedef struct nodo ListaJugadores;

ParametrosMapa leerParametrosConsola(char** argv)
{
	ParametrosMapa parametros;
	parametros.nombreMapa = argv[1];
	parametros.dirPokedex = argv[2];

	return parametros;
}

void verificarParametros(int argc)
{
	if(argc!=3)
	{
		printf("Error - Faltan parametros \n");
		exit(1);
	}
}

MetadataMapa leerMetadataMapa()
{
	MetadataMapa mdata;
	t_config* config; //Estructura
	char* auxiliar;


	//RUTA ABSOLUTA
	//config = config_create("//home/utnso/SistOp/tp-2016-2c-Breaking-Bug/Proc-Mapa/config/mapa.config");
	//RUTA RELATIVA
	config = config_create("../config/mapa.config");

	if(config==NULL)
	{
		printf("Archivo mapa.config no encontrado\n");
		exit(20);
	}

	mdata.tiempoChequeoDeadlock = config_get_int_value(config, "TiempoChequeoDeadlock");
	mdata.modoBatalla = config_get_int_value(config, "Batalla");
	mdata.quantum = config_get_int_value(config,"quantum");
	mdata.retardo = config_get_int_value(config,"retardo");

	//Esta parte es para no perder la referencia de los punteros al hacer config_destroy

	auxiliar = config_get_string_value(config,"algoritmo");
	mdata.algoritmo = malloc(strlen(auxiliar)+1);
	strcpy(mdata.algoritmo, auxiliar);

	auxiliar = config_get_string_value(config,"IP");
	mdata.ip = malloc(strlen(auxiliar)+1);
	strcpy(mdata.ip, auxiliar);

	auxiliar = config_get_string_value(config,"Puerto");
	mdata.puerto = malloc(strlen(auxiliar)+1);
	strcpy(mdata.puerto, auxiliar);

	config_destroy(config);

	return mdata;
}


MetadataPokenest leerMetadataPokenest()
{
	MetadataPokenest mdata;
	t_config* config; //Estructura

	char* auxiliar;

	//RUTA ABSOLUTA
	//config = config_create("/home/utnso/SistOp/tp-2016-2c-Breaking-Bug/Proc-Mapa/config/pokenest.config");
	//RUTA RELATIVA
	config = config_create("../config/pokenest.config");

	if(config==NULL)
	{
		printf("Archivo pokenest.config no encontrado\n");
		exit(20);
	}

	auxiliar = config_get_string_value(config, "Posicion");


	//Procesamiento de posicion de string a dos ints
	int i = strlen(auxiliar)-1;
	int pos_es_y = 1;
	int potencia = 0;
	mdata.posicionX = 0;
	mdata.posicionY = 0;

	for (;i>=0;i--)
	{
		if (isdigit(auxiliar[i]))
		{
			if (pos_es_y)
				mdata.posicionY += (auxiliar[i]-'0') * (int)powf(10,potencia);
			else
				mdata.posicionX += (auxiliar[i]-'0') * (int)powf(10,potencia);
			potencia++;
		}
		else
		{
			pos_es_y = 0;
			potencia = 0;
		}
	}

	auxiliar = config_get_string_value(config, "Tipo");
	mdata.tipoPokemon = malloc(strlen(auxiliar)+1);
	strcpy(mdata.tipoPokemon, auxiliar);

	auxiliar = config_get_string_value(config,"Identificador");
	mdata.identificador = malloc(strlen(auxiliar)+1);
	strcpy(mdata.identificador, auxiliar);


	config_destroy(config);

	return mdata;
}

MetadataPokemon leerMetadataPokemon()
{
	MetadataPokemon mdata;
	t_config* config; //Estructura

	//RUTA ABSOLUTA
	//config = config_create("/home/utnso/SistOp/tp-2016-2c-Breaking-Bug/Proc-Mapa/config/pokemon.config");
	//RUTA RELATIVA
	config = config_create("../config/pokemon.config");

	if(config==NULL)
	{
		printf("Archivo pokemon.config no encontrado\n");
		exit(20);
	}

	mdata.nivel = config_get_int_value(config, "Nivel");

	config_destroy(config);

	return mdata;
}


void init_nivel()
{
	nivel_gui_inicializar();
}

void inicializar_entrenador(Entrenador* entrenador)
{
    entrenador->posx = 1;
    entrenador->posy = 1;
    entrenador->simbolo = '@';
    entrenador->movAnterior = 'y';
    entrenador->flagx = FALSE;
    entrenador->flagy = FALSE;
    entrenador->pokemones = NULL;
}

void inicializar_jugador(Jugador* unJugador, int unSocket){
	inicializar_entrenador(&(unJugador->entrenador));
	unJugador->socket = unSocket;
	unJugador->estado = 0;
}

int agregar_a_lista (ListaJugadores* lista, Jugador jugador)
{
	ListaJugadores* aux;
	aux=lista;
	while(aux->sgte!=NULL)
		aux=aux->sgte;

	if (aux==lista && lista->jugador.socket==0)
		lista->jugador=jugador;
	else
	{
		if((aux->sgte=malloc(sizeof(ListaJugadores)))==NULL)
			return 1;
		aux->sgte->jugador=jugador;
		aux->sgte->sgte=NULL;
	}
	return 0;
}

int lista_tiene_jugador(ListaJugadores lista, int socket)
{
	if (lista.jugador.socket == socket) return 1;
	while(lista.sgte!=NULL)
	{
		lista=*(lista.sgte);
		if(lista.jugador.socket == socket) return 1;

	}
	return 0;
}

void agregar_si_no_existe(ListaJugadores *lista, Jugador jugador)
{
	if (!lista_tiene_jugador(*lista, jugador.socket))
		agregar_a_lista(lista, jugador);
}

void loggearColas(void){
	t_queue *auxLista;
	auxLista = queue_create();
	Jugador* jugador;
	if (!queue_is_empty(colaListos)){
		auxLista = colaListos;
		log_info(infoLogger, "Los Entrenadores que se encuentran en la cola de listos son:\n");
		while(auxLista!=NULL){
			jugador = (Jugador*)queue_pop(auxLista);
			log_info(infoLogger, "%s \n", jugador->entrenador);
		}
	}

	if (!queue_is_empty(colaBloqueados)){
			auxLista = colaBloqueados;
			log_info(infoLogger, "Los Entrenadores que se encuentran en la cola de bloqueados son:\n");
			while(auxLista!=NULL){
				jugador = (Jugador*)queue_pop(auxLista);
				log_info(infoLogger, "%s \n", jugador->entrenador);
			}
		}
}

//ESTE ES EL HILO PLANIFICADOR !!!! :D. Escribí aca directamente el codigo, en el main ya estan las instrucciones para ejecutarlo
void* thread_planificador()
{
	ListaJugadores jugadoresConectados;
	jugadoresConectados.sgte=NULL;
	Jugador* jugadorPlanificado;

	int quantum = 4; //TODO leer del archivo metadata.
	int i=0;
	char* buffer;

	//init_nivel();

	while(1)
	{
		jugadorPlanificado = (Jugador*)queue_pop(colaListos);
	    //log_info(infoLogger, "El entrenador %s con el ip %d ha salido de la cola de listos."),
	    	//(jugadorPlanificado)-> entrenador, (jugadorPlanificado)-> estado;
	    		//, (jugadorPlanificado)-> estado, (jugadorPlanificado)->socket);
		//loggearColas();

		for(i=0; i<quantum;i++)
		{
			if (jugadorPlanificado != NULL && recv(jugadorPlanificado->socket, buffer, TAMANIO_BUFFER, 0)!=0) //No hay error de conexion
			{
				if (buffer[0]=='0') //Handshake
				{
					if (!lista_tiene_jugador(jugadoresConectados, jugadorPlanificado->socket))
					{
						printf("Detecto nuevo socket. Mensaje: %s\n", buffer);
						//TODO Leer datos del buffer y completar jugadorPlanificado.
					}
				}
				else
				{
					switch(buffer[0])
					{
						case '1':
							buffer++;
							break;
							case '2':/*
								buffer++;
								if (buffer[0]=='x')
									jugadorPlanificado->entrenador.posx++;
								else
									jugadorPlanificado->entrenador.posy++;*/
								break;
							case '3':
								buffer++;
								break;
							case '4':
								buffer++;
								break;
					}
				}
			}
			else
			{
				//TODO cerrar conexion.
			}
		}
		//nivel_gui_dibujar(listaDibujo, "Pueblo Paleta");
	}
}


int main(int argc, char** argv)
{
	/*
	ParametrosMapa parametros;
	verificarParametros(argc); //Verificamos que la cantidad de Parametros sea correcta
	parametros = leerParametrosConsola(argv); //Leemos parametros por Consola

	printf("Nombre Mapa: %s --- Dir Pokedex: %s \n",parametros.nombreMapa, parametros.dirPokedex);
*/

	traceLogger = log_create("Logs.log", "Mapa", false, LOG_LEVEL_TRACE);
	infoLogger = log_create("Logs.log", "Mapa", false, LOG_LEVEL_INFO);
	log_info(infoLogger, "Se inicia Mapa.");

	MetadataPokenest mdataPokenest;
	MetadataMapa mdataMapa;
	MetadataPokemon mdataPokemon;


	mdataMapa = leerMetadataMapa();
	mdataPokenest = leerMetadataPokenest();
	mdataPokemon = leerMetadataPokemon();

	//**********************************
	//ToDo FALTAN LEER LOS ARCHIVOS DE CONFIGURACION DE POKEMON Y POKENEST, YA ESTAN LAS ESTRUCTURAS DEFINIDAS EN EL HEADER!

	//**********************************

	/*
	printf("\nDatos Mapa ---------\n");
	printf("Tiempo chequeo deadlock %d\n", mdataMapa.tiempoChequeoDeadlock);
	printf("Batalla %d\n", mdataMapa.modoBatalla);
	printf("Quantum %d\n", mdataMapa.quantum);
	printf("Retardo %d\n", mdataMapa.retardo);
	printf("IP %s\n", mdataMapa.ip);
	printf("Puerto %s\n", mdataMapa.puerto);
	printf("\nDatos Pokenest ----------\n");
	printf("Identificador: %s\n",mdataPokenest.identificador);
	printf("Tipo: %s\n",mdataPokenest.tipoPokemon);
	printf("Posicion X: %d\n",mdataPokenest.posicionX);
	printf("Posicion Y: %d\n",mdataPokenest.posicionY);
	printf("\nDatos Pokemon ----------\n");
	printf("Nivel: %d\n",mdataPokemon.nivel);
	*/

	//Para crear una entrada en un archivo LOG:
	//log_tipoDeLog (logger, "mensaje"). tipoDeLog = trace, info, error, etc

	//socket_startServer();

	//**********************************
	//**********************************
	//FUNCION SERVER

	fd_set fds_entrenadores;   // conjunto maestro de descriptores de fichero
	fd_set read_fds; // conjunto temporal de descriptores de fichero para select()

	int fdmax;        // número máximo de descriptores de fichero
	int listener;     // descriptor de socket a la escucha
	void* buf;
	int nbytes;
	int i, j;
	FD_ZERO(&fds_entrenadores);    // borra los conjuntos maestro y temporal
	FD_ZERO(&read_fds);

	listener = socket_startListener(atoi(mdataMapa.puerto));

	// añadir listener al conjunto maestro
	FD_SET(listener, &fds_entrenadores);

	// seguir la pista del descriptor de fichero mayor
	fdmax = listener; // por ahora es éste

	// bucle principal

	colaListos = queue_create();
	//colaBloqueados = queue_create();


	//FALTAN CARGAR LAS POKENEST Y DIBUJARLAS

	pthread_t hiloPlanificador;
	int valorHilo;

	valorHilo = pthread_create(&hiloPlanificador,NULL,thread_planificador,NULL);

	if(valorHilo != 0)
	{
		perror("Error al crear hilo Planificador");
		exit(1);
	}

	Jugador nuevoJugador;
	int newfd;

	for (;;) {
		getch();
		read_fds = fds_entrenadores; // cópialo

		//Buscamos los sockets que quieren realizar algo con Select
		socket_select(fdmax, &read_fds);

		// explorar conexiones existentes en busca de datos que leer
		for(i = 0; i <= fdmax; i++) {

			if (FD_ISSET(i, &read_fds)) { // ¡¡tenemos datos!!

				if (i == listener) {
					//SE ACEPTA UN NUEVO ENTRENADOR
					newfd = socket_addNewConection(listener,&fds_entrenadores,&fdmax);
					inicializar_jugador(&nuevoJugador, newfd);
					queue_push(colaListos, &nuevoJugador);
					log_info(infoLogger, "%s ha ingresado a la cola de listos con ip %d y el socket %d.",
					(&nuevoJugador)-> entrenador, (&nuevoJugador)-> estado, (&nuevoJugador)->socket);
					loggearColas();
				}
					//A PARTIR DE ACA SE RECIBEN DATOS DEL CLIENTE
				else {
					queue_push(colaListos, &nuevoJugador);
					log_info(infoLogger, "%s ha ingresado a la cola de listos.", (&nuevoJugador) -> entrenador);
					loggearColas();
					/* gestionar datos de un cliente
					if ((nbytes = recv(i, buf,200, 0)) <= 0) { // error o conexión cerrada por el cliente
						if (nbytes == 0) { //EL ENTRENADOR SE DESCONECTO
							socket_closeConection(i,&fds_entrenadores);
							}

						else {
							perror("recv");
						}

					}
					else {
						// tenemos datos de algún cliente
					}
						*/
				} // Esto es ¡TAN FEO!
			}
		}


			//nivel_gui_dibujar(listaDibujo, v);
			}


	free(mdataPokenest.identificador);
	free(mdataPokenest.tipoPokemon);
	free(mdataMapa.algoritmo);
	free(mdataMapa.ip);
	free(mdataMapa.puerto);

	log_info(infoLogger, "Se cierra Mapa.");
	log_destroy(traceLogger);
	log_destroy(infoLogger);
	return 0;
}
