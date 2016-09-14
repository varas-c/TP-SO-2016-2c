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
#include "headers/configMapa.h"
#include "headers/serializeMapa.h"

/****************************************************************************************************************
 * ************************************************************************************************************
			VARIABLES GLOBALES
****************************************************************************************************************
****************************************************************************************************************/


#define TAMANIO_BUFFER 11
t_queue* colaListos;
t_queue* colaBloqueados;
t_queue* colaDesconectados;
t_list* gui_items;
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

t_list* listaPokenest;



/****************************************************************************************************************
 * ************************************************************************************************************
 *
			FUNCIONES DE LECTURA DE PARAMETROS POR CONSOLA (LOS QUE SE RECIBEN POR **ARGV)

****************************************************************************************************************
****************************************************************************************************************/

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



/****************************************************************************************************************
 * ************************************************************************************************************
 *
****************************************************************************************************************
****************************************************************************************************************/


void loggearColas(void){
	t_queue *auxLista;
	auxLista = queue_create();
	Jugador* jugador;
	if (!queue_is_empty(colaListos)){
		auxLista = colaListos;
		log_info(infoLogger, "Los Entrenadores que se encuentran en la cola de listos son:\n");
		while(auxLista!=NULL){
			jugador = (Jugador*)queue_pop(auxLista);
			log_info(infoLogger, "%s \n", jugador->entrenador); //esto me muestra todos los que estan la cola
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

/****************************************************************************************************************
 * ************************************************************************************************************
 *
****************************************************************************************************************
****************************************************************************************************************/

void free_paquete(Paquete *paquete)
{
	free(paquete->buffer);
	paquete->tam_buffer = -1;
}



void send_turno(Paquete* paquete,int socket)
{
	send(socket,paquete->buffer,paquete->tam_buffer,0);
}

MetadataPokenest buscar_Pokenest(char simbolo)
{
	bool _find_pokenest_(MetadataPokenest* aux)
	{
		return aux->simbolo == simbolo;
	}

	MetadataPokenest *ptr = (MetadataPokenest*) list_find(listaPokenest,(void*)_find_pokenest_);

	MetadataPokenest pokenest;
	pokenest.simbolo = ptr->simbolo;
	pokenest.posicionX = ptr->posicionX;
	pokenest.posicionY = ptr->posicionY;
	pokenest.tipoPokemon = strdup(ptr->tipoPokemon);

	return pokenest;
}

void send_Pokenest(int socket,Paquete *paquete)
{
	send(socket,paquete->buffer,paquete->tam_buffer,0);
}

void send_Turno(int socket)
{
	Paquete paquete = srlz_turno();
	send_turno(&paquete,socket);
	free_paquete(&paquete);
}

void movEntrenador(PosEntrenador pos, Jugador* jugador)
{
	jugador->entrenador.posx = pos.x;
	jugador->entrenador.posy = pos.y;

}

char recv_simboloEntrenador(int socket)
{
	char simbolo;

	Paquete paquete;
	paquete.buffer = malloc(size_SIMBOLO);
	paquete.tam_buffer = size_SIMBOLO;


	recv(socket,paquete.buffer,paquete.tam_buffer,0);

	simbolo = dsrlz_simboloEntrenador(paquete.buffer);

	return simbolo;
}

/****************************************************************************************************************
 * ************************************************************************************************************
 *
****************************************************************************************************************
****************************************************************************************************************/

//ESTE ES EL HILO PLANIFICADOR !!!! :D. Escribí aca directamente el codigo, en el main ya estan las instrucciones para ejecutarlo
void* thread_planificador()
{
	Paquete paquete;

	//Hay que sacar a todos los que se fueron y nos avisó el mapa!
	void* buffer_recv;
	int tam_buffer_recv = 100;

	Jugador *jugador;
	Jugador *jugador2;
	int socket_desconectado;
	Jugador *jugadorDesconectado;
	int quantum = mdataMapa.quantum;
	int codOp = -1;
	char pokenestPedida;
	MetadataPokenest pokenestEnviar;
	char* mostrar = malloc(100);

	PosEntrenador pos;

	while(1)
	{

	usleep(mdataMapa.retardo*1000);

	pthread_mutex_lock(&mutex_Desconectados);
	while(!queue_is_empty(colaDesconectados))
	{
		//ELIMINAMOS JUGADORES
		//socket_desconectado = (int) queue_pop(colaDesconectados);
	}
	pthread_mutex_unlock(&mutex_Desconectados);

	//Si nadie mas se quiere ir, es hora de Jugar!

	int flag = 0;

	//TODO poner semaforo contador verificando lista vacia
	if(!queue_is_empty(colaListos))
	{

		buffer_recv = malloc(tam_buffer_recv);
		flag = 1;

		jugador = queue_pop(colaListos);

		//log_info(infoLogger, "%s se ha ido de la cola de listos con ip %d y el socket %d.",(&jugador)-> entrenador, (&jugador)-> estado, (&jugador)->socket);
		//loggearColas();
		//socket_bloqueado = jugador->socket;

		//Ya tenemos jugador, ahora le mandamos un turno
		send_Turno(jugador->socket);

		//Ya mandamos el turno, ahora recibimos el pedido del entrenador
		recv(jugador->socket,buffer_recv,tam_buffer_recv,0);

		//Tomamos el primer int del buffer para ver el código de operacion
		codOp = dsrlz_codigoOperacion(buffer_recv);

		//02 - X - Y

		//Evaluo el codigo de Operacion para ver que verga quiere
		switch(codOp)
		{
		case POKENEST: //Nos pidieron una pokenest, hay que entregarla:
			pokenestPedida = dsrlz_Pokenest(buffer_recv); //Obtenemos el simbolo de la pokenest que nos pidieron
			pokenestEnviar = buscar_Pokenest(pokenestPedida); //Buscamos la info de la Pokenest pedida
			paquete = srlz_Pokenest(pokenestEnviar); //Armamos un paquete serializado
			send_Pokenest(jugador->socket,&paquete); //Enviamos el paquete :D
			free_paquete(&paquete);//Liberamos el paquete
		break;
		case MOVER: //El entrenador se quiere mover
			pos = dsrlz_movEntrenador(buffer_recv); //Obtengo las coordenadas X,Y
			movEntrenador(pos,jugador);//Actualizamos el entrenador con las nuevas coordenadas
			MoverPersonaje(gui_items, jugador->entrenador.simbolo, jugador->entrenador.posx, jugador->entrenador.posy);
			break;

		case CAPTURAR: //TODO: FALTA COMPLETAR!!
			pokenestPedida = dsrlz_Pokenest(buffer_recv);//Identificamos la pokenest pedida
			//Hay que preguntar si tenemos recursos
			//Si tenemos recursos, hay que mandarle un mensaje para que copie el archivo!
			//Si no tenemos recursos hay que bloquearlo!!!
		break;
		case FINOBJETIVOS:
			//TODO:
			break;



		}


		sprintf(mostrar,"Jugador: %i",jugador->socket);

		free(buffer_recv);

		//pthread_mutex_lock(&mutex_socket);

		pthread_mutex_lock(&mutex_Listos);
		queue_push(colaListos,(void*)jugador);
		log_trace(traceLogger, "Termina turno de jugador %c", jugador->entrenador.simbolo);
		pthread_mutex_unlock(&mutex_Listos);
		//log_info(infoLogger, "Jugador %c entra en Cola Listos", jugador->entrenador.simbolo);

		//pthread_mutex_unlock(&mutex_socket);

		//log_info(infoLogger, "%s ha ingresado a la cola de listos con ip %d y el socket %d.",(&jugador)-> entrenador, (&jugador)-> estado, (&jugador)->socket);
		//loggearColas();
		//pthread_mutex_lock(&mutex_socket);
	}

	nivel_gui_dibujar(gui_items, mostrar);

	}
}

/****************************************************************************************************************
 * ************************************************************************************************************
 *
****************************************************************************************************************
****************************************************************************************************************/

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

	//Inicializamos espacio de dibujo
	nivel_gui_inicializar();

	gui_items = list_create();


	listaPokenest= list_create();

	mdataMapa = leerMetadataMapa();
	mdataPokenest = leerMetadataPokenest();
	mdataPokemon = leerMetadataPokemon();

	//Agrego a la lista
	list_add(listaPokenest,&mdataPokenest);

	//Agrego a la lista de dibujo
	pthread_mutex_lock(&mutex_gui_items);
	CrearCaja(gui_items, mdataPokenest.simbolo, mdataPokenest.posicionX, mdataPokenest.posicionY,6);
	pthread_mutex_unlock(&mutex_gui_items);

	//**********************************!

	//**********************************

	//Para crear una entrada en un archivo LOG:
	//log_tipoDeLog (logger, "mensaje"). tipoDeLog = trace, info, error, etc

	//**********************************
	//**********************************
	//FUNCION SERVER

	fd_set read_fds; // conjunto temporal de descriptores de fichero para select()
	fd_set fds_entrenadores;   // conjunto maestro de descriptores de fichero

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
	colaDesconectados = queue_create();
	colaBloqueados = queue_create();

	//FALTAN CARGAR LAS POKENEST Y DIBUJARLAS

	int valor_recv;
	void *buffer_recv;
	int tamBuffer_recv = 10;
	buffer_recv = malloc(tamBuffer_recv);

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
	int newfd;
	char simboloEntrenador;


	for (;;) {
		//getch();
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
					nuevoJugador = new_Jugador(simboloEntrenador,newfd);

					aux = malloc(sizeof(Jugador)); //NO HACERLE FREE !!!!!!!!!
					aux->entrenador = nuevoJugador.entrenador;
					aux->socket = nuevoJugador.socket;
					aux->estado = nuevoJugador.estado;
					CrearPersonaje(gui_items,nuevoJugador.entrenador.simbolo,nuevoJugador.entrenador.posx, nuevoJugador.entrenador.posy);
					pthread_mutex_lock(&mutex_Listos);
					queue_push(colaListos, aux);
					pthread_mutex_unlock(&mutex_Listos);
					log_info(infoLogger, "Nuevo jugador: %c, socket %d", nuevoJugador.entrenador.simbolo, nuevoJugador.socket);
					//log_info(infoLogger, "%s ha ingresado a la cola de listos con ip %d y el socket %d.",
					//(&nuevoJugador)-> entrenador, (&nuevoJugador)-> estado, (&nuevoJugador)->socket);
					//loggearColas();

				}
				//Si no es el Listener, el entrenador SE DESCONECTÓ!!
				else
				{

					valor_recv = recv(i, buffer_recv, tamBuffer_recv, 0);

					if(valor_recv == 0)
					{
						//TODO Completar
						//pthread_mutex_lock(&mutex_Desconectados);
						//queue_push(colaDesconectados,&i);
						//pthread_mutex_unlock(&mutex_Desconectados);
						//log_info(infoLogger, "Detectada desconexion de socket %d", i);
					}

				}
			}
		}

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
