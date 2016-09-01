/*
 * mapa.c
 *
 *  Created on: 30/8/2016
 *      Author: utnso
 */
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

//	config = config_create("/home/utnso/SistOp/tp-2016-2c-Breaking-Bug/Proc-Mapa/config/mapa.config");
	config = config_create("config/mapa.config");
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

//	config = config_create("/home/utnso/SistOp/tp-2016-2c-Breaking-Bug/Proc-Mapa/config/pokenest.config");
	config = config_create("config/pokenest.config");
	mdata.posicion = config_get_int_value(config, "Posicion");

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

	//config = config_create("/home/utnso/SistOp/tp-2016-2c-Breaking-Bug/Proc-Mapa/config/pokemon.config");
	config = config_create("config/pokemon.config");
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
    entrenador -> flagx = FALSE;
    entrenador -> flagy = FALSE;
}

void inicializar_jugador(Jugador* unJugador, int unSocket){
	inicializar_entrenador(&(unJugador->entrenador));
	unJugador->socket = unSocket;
	unJugador->estado = 0;
}


void socket_startServer()
{
	fd_set fds_entrenadores;   // conjunto maestro de descriptores de fichero
	fd_set read_fds; // conjunto temporal de descriptores de fichero para select()

	int fdmax;        // número máximo de descriptores de fichero
	int listener;     // descriptor de socket a la escucha
	void* buf;
	int nbytes;
	int i, j;
	FD_ZERO(&fds_entrenadores);    // borra los conjuntos maestro y temporal
	FD_ZERO(&read_fds);

	listener = socket_startListener();

	// añadir listener al conjunto maestro
	FD_SET(listener, &fds_entrenadores);

	// seguir la pista del descriptor de fichero mayor
	fdmax = listener; // por ahora es éste

		// bucle principal

	t_queue* colaListos;
	colaListos = queue_create();

	t_list* listaDibujo;
	listaDibujo = list_create();
	init_nivel();

	//FALTAN CARGAR LAS POKENEST Y DIBUJARLAS

	nivel_gui_dibujar(listaDibujo, "");

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
					CrearPersonaje(listaDibujo, nuevoJugador.entrenador.simbolo,nuevoJugador.entrenador.posx, nuevoJugador.entrenador.posy);
					queue_push(colaListos, &nuevoJugador);
					//Aca se debe crear un nuevo struct entrenador y crear la interfaz gráfica.
				}

				//A PARTIR DE ACA SE RECIBEN DATOS DEL CLIENTE
				else {
					buf = malloc(200);
					// gestionar datos de un cliente
					if ((nbytes = recv(i, buf,200, 0)) <= 0) { // error o conexión cerrada por el cliente
						if (nbytes == 0) { //EL ENTRENADOR SE DESCONECTO
							socket_closeConection(i,&fds_entrenadores);
							}

							else {
								perror("recv");
							}

						}

					/*
					else {

						// tenemos datos de algún cliente
						for (j = 0; j <= fdmax; j++) {
							// ¡enviar a todo el mundo!
							if (FD_ISSET(j, &master)) {
								// excepto al listener y a nosotros mismos
								if (j != listener && j != i) {
									if (send(j, buf, 100, 0) == -1) {
										perror("send");
										}
									}
								}
							}

						}

					*/

					} // Esto es ¡TAN FEO!
				}
			}
		int tamanio = queue_size(colaListos);
		char v[2];
		sprintf(v, "%i", tamanio);
		nivel_gui_dibujar(listaDibujo, v);
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


	MetadataPokenest mdataPokenest;
	MetadataMapa mdataMapa;
	MetadataPokemon mdataPokemon;


	mdataMapa = leerMetadataMapa();
	mdataPokenest = leerMetadataPokenest();
	mdataPokemon = leerMetadataPokemon();


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
	printf("Posicion: %d\n",mdataPokenest.posicion);
	printf("\nDatos Pokemon ----------\n");
	printf("Nivel: %d\n",mdataPokemon.nivel);

	free(mdataPokenest.identificador);
	free(mdataPokenest.tipoPokemon);
	free(mdataMapa.algoritmo);
	free(mdataMapa.ip);
	free(mdataMapa.puerto);
*/

	socket_startServer();
	printf("\n\n");

	return 0;

}
