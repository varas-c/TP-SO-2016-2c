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
#include <sys/ioctl.h>
#include <curses.h>
#include <math.h>
#include <ctype.h>
#include <nivel.h>
#include <commons/log.h>
#include <commons/collections/queue.h>
#include <commons/config.h>
#include <commons/collections/list.h>
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

void send_Turno(int socket)
{
	Paquete paquete = srlz_turno();
	send(socket,paquete.buffer,paquete.tam_buffer,0);
	free_paquete(&paquete);
}
//****************************************************************************************************************

Jugador *buscarJugadorPorSocket(t_queue* colaListos, int socketBuscado){
	t_queue* cola_auxiliar = queue_create();
	Jugador *auxiliar, *auxiliar2, *encontrado;
	int tamanio = queue_size(colaListos);
	int tamanio2;
	while(tamanio > 0){
		auxiliar = malloc(sizeof(Jugador));
		auxiliar = queue_pop(colaListos);
		if(auxiliar->socket != socketBuscado){
			queue_push(cola_auxiliar, auxiliar);
		}else{
			encontrado = auxiliar;
		}
		tamanio = tamanio-1;
		}
		tamanio2 = queue_size(cola_auxiliar);
		while(tamanio2 > 0){
			auxiliar2 = malloc(sizeof(Jugador));
			auxiliar2 = queue_pop(cola_auxiliar);
			queue_push(colaListos, auxiliar2);
			tamanio2 = tamanio2 - 1;
		}
	free(auxiliar);
	free(auxiliar2);
	return encontrado;
}

//****************************************************************************************************************
void* thread_planificador() //ESTE ES EL HILO PLANIFICADOR !!!! :D.
{							//Escribí aca directamente el codigo, en el main ya estan las instrucciones
	Paquete paquete;		// para ejecutarlo
	//Hay que sacar a todos los que se fueron y nos avisó el mapa!
	void* buffer_recv;
	int tam_buffer_recv = 100;
	int estado_socket;
	Jugador *jugador;
	Jugador *jugadorAux;
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
		jugadorDesconectado = malloc(sizeof(Jugador));
		socket_desconectado = queue_pop(colaDesconectados);
		pthread_mutex_lock(&mutex_Listos);
		jugadorDesconectado = buscarJugadorPorSocket(colaListos, socket_desconectado);
		pthread_mutex_unlock(&mutex_Listos);
		BorrarItem(gui_items,jugadorDesconectado->entrenador.simbolo);
		free(jugadorDesconectado);
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

		log_info(infoLogger, "Jugador %c sale de Listos.",jugador->entrenador.simbolo);
		loggearColas();
		//socket_bloqueado = jugador->socket;

		//Ya tenemos jugador, ahora le mandamos un turno
		send_Turno(jugador->socket);

		//Ya mandamos el turno, ahora recibimos el pedido del entrenador
		estado_socket = recv(jugador->socket,buffer_recv,tam_buffer_recv,0);

		if(estado_socket == 0){
			jugadorAux = malloc(sizeof(Jugador));
			jugadorAux = jugador;
			BorrarItem(gui_items,jugadorAux->entrenador.simbolo);
			close(jugadorAux->socket);
			free(jugadorAux);
		}else{
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
			pos = dsrlz_movEntrenador(buffer_recv);
			//Obtengo las coordenadas X,Y
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
		}
		sprintf(mostrar,"Jugador: %i",jugador->socket);

		free(buffer_recv);

		//pthread_mutex_lock(&mutex_socket);

		pthread_mutex_lock(&mutex_Listos);
		queue_push(colaListos,(void*)jugador);
		log_trace(traceLogger, "Termina turno de jugador %c", jugador->entrenador.simbolo);
		pthread_mutex_unlock(&mutex_Listos);
		log_info(infoLogger, "Jugador %c entra en Cola Listos", jugador->entrenador.simbolo);
		loggearColas();

		//pthread_mutex_unlock(&mutex_socket);

		//log_info(infoLogger, "%s ha ingresado a la cola de listos con ip %d y el socket %d.",(&jugador)-> entrenador, (&jugador)-> estado, (&jugador)->socket);
		//loggearColas();
		//pthread_mutex_lock(&mutex_socket);
	}

	nivel_gui_dibujar(gui_items, mostrar);

	}
}
//****************************************************************************************************************

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

	//**********************************

	//Para crear una entrada en un archivo LOG:
	//log_tipoDeLog (logger, "mensaje"). tipoDeLog = trace, info, error, etc

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
	int *aux2;
	int newfd;
	char simboloEntrenador;

	for (;;) {
		getch();
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
					log_info(infoLogger, "Jugador %c entra en Cola Listos", nuevoJugador.entrenador.simbolo);
					loggearColas();
				}
				//Si no es el Listener, el entrenador SE DESCONECTÓ!!
				else
				{
					valor_recv = recv(i, buffer_recv, tamBuffer_recv, 0);

					if(valor_recv == 0)
					{
						//TODO Completar
						close(i);
						pthread_mutex_lock(&mutex_Desconectados);
						aux2 = malloc(sizeof(int));
						*aux2 = i;
						queue_push(colaDesconectados,aux2);
						//queue_push(colaDesconectados,&i);
						pthread_mutex_unlock(&mutex_Desconectados);
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
