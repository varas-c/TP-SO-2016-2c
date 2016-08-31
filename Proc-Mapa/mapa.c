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

	config = config_create("/home/utnso/SistOp/tp-2016-2c-Breaking-Bug/Proc-Mapa/config/mapa.config");

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

	config = config_create("/home/utnso/SistOp/tp-2016-2c-Breaking-Bug/Proc-Mapa/config/pokenest.config");

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

	config = config_create("/home/utnso/SistOp/tp-2016-2c-Breaking-Bug/Proc-Mapa/config/pokemon.config");

	mdata.nivel = config_get_int_value(config, "Nivel");

	config_destroy(config);

	return mdata;
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
	//PARA HACER: FALTAN LEER LOS ARCHIVOS DE CONFIGURACION DE POKEMON Y POKENEST, YA ESTAN LAS ESTRUCTURAS DEFINIDAS EN EL HEADER!

	nivel_gui_inicializar();
	t_list* lista=list_create();
	ITEM_NIVEL cosa;
	cosa.id='Z';
	cosa.item_type='P';
	cosa.posx=45;
	cosa.posy=12;
	cosa.quantity=1;
//	lista.head=malloc(8);
//	lista.head->data=&cosa;
//	lista.head->next=NULL;
	list_add(lista, &cosa);
	int j=0;
	for(j=0;j<32766;j++)
		nivel_gui_dibujar(lista, "Prueba");
	nivel_gui_terminar();

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

	socket_startServer();
	printf("\n\n");

	return 0;

}
