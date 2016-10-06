/*
 * entrenador.c
 *
 *  Created on: 29/8/2016
 *      Author: utnso
 */

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <commons/config.h>
#include <commons/log.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include "headers/struct.h"
#include "headers/socket.h"
#include "headers/configEntrenador.h"
#include "headers/send.h"
#include "headers/serializeEntrenador.h"

int fd_server;
int vidas_restantes;

char* concatObjetivo(char* ciudad, char* obj)//El calculo es 4 de "obj[" + largociudad + "]" + /n
{
	obj = malloc(4*sizeof(char)+strlen(ciudad)*sizeof(char)+2);

	strcat(obj,"obj[");
	strcat(obj,ciudad);
	strcat(obj,"]");

	return obj;
}
//******************************************

void mostrarObjetivos (char **a)//Muestra los objetivos de un mapa
{
	int cantViajes=0;

	printf("Objetivos Mapa: ");
	while(a[cantViajes] != '\0')
	{
			printf("%s",a[cantViajes]);
			cantViajes++;
	}
	printf("\n");
}

char getNumObjetivo(int numero, char** objetivos)
{
	return objetivos[numero];

}
//******************************************

int cantidadDeViajes(char** hojaDeViaje)//Cuenta la cantidad de viajes en una hoja de Viaje
{
	int cantViajes=0;

	while(hojaDeViaje[cantViajes] != '\0')
	{
			cantViajes++;
	}

	return cantViajes;
}

/*
void leerObjetivos(char* objetivos, t_config* config, int cantViajes,char** hojaDeViaje)
{
	char* ciudad;
	char* leer;
	char** a;

	int i;
	for(i=0;i<cantViajes;i++)
	{
		ciudad = hojaDeViaje[i];
		leer = concatObjetivo(ciudad);
		objetivos[i] = config_get_array_value(config,leer);
		mostrarObjetivos(a);
		free(leer);
	}
}
*/

//******************************************

void mover_entrenador(Entrenador *entrenador)//Si flagx=false then mover en X otherwise mover en Y
{
	int move = 0;

	if(entrenador->destinox != 0 && entrenador->movAnterior == 'y' && move == 0)
	{
		if(entrenador->destinox < 0 ) //Me muevo hacia atras en X
		{
			entrenador->posx -=1;
			entrenador->destinox += 1;
		}

		if(entrenador->destinox > 0)
		{
			entrenador->posx += 1;
			entrenador->destinox -= 1;
		}

		entrenador->movAnterior = 'x';
		move = 1;
	}

	if(entrenador->destinoy != 0 && entrenador->movAnterior == 'x' && move == 0)
	{
		if(entrenador->destinoy < 0 ) //Me muevo hacia atras en X
		{
			entrenador->posy -=1;
			entrenador->destinoy += 1;
		}

		if(entrenador->destinoy > 0)
		{
			entrenador->posy += 1;
			entrenador->destinoy -= 1;
		}

		entrenador->movAnterior = 'y';
		move = 1;
	}

	if(entrenador->destinox != 0 && move == 0) //Si todavia me queda mvimiento en X
	{
		if(entrenador->destinox < 0 ) //Me muevo hacia atras en X
		{
			entrenador->posx -=1;
			entrenador->destinox += 1;
		}

		if(entrenador->destinox > 0)
		{
			entrenador->posx += 1;
			entrenador->destinox -= 1;
		}

		entrenador->movAnterior = 'x';
		move = 1;
	}

	if(entrenador->destinoy != 0 && move == 0)
	{
		if(entrenador->destinoy < 0 ) //Me muevo hacia atras en X
		{
			entrenador->posy -=1;
			entrenador->destinoy += 1;
		}

		if(entrenador->destinoy > 0)
		{
			entrenador->posy += 1;
			entrenador->destinoy -= 1;
		}

		entrenador->movAnterior = 'y';
		move = 1;
	}
}

int recv_turnoConcedido(int fd_server)
{
	int tam_buffer = sizeof(int);
	void* mensaje[tam_buffer];
	int codigo;

	int valor_recv = recv(fd_server, (void*)mensaje, tam_buffer, 0);

	if(valor_recv == -1)
	{
		perror("Error recv");
		exit(1);
	}
	//Deserializamos
	memcpy(&codigo,mensaje,sizeof(int));

	if(codigo == TURNO)
	{
		return 1;
	}
	return 0;
}
//****************************************

int faltaPokenest(Pokenest pokenest)//Si la pokenest tiene posx y posy = -1 es porque no tenemos datos,hay que leer
{
	return pokenest.posx == -1 || pokenest.posy == -1;
}
//****************************************//****************************************

int llegueAPokenest(const Entrenador entrenador, const Pokenest pokenest)
{
	if(entrenador.posx == pokenest.posx && entrenador.posy == pokenest.posy)
	{
		return 1;
	}
	return 0;
}
//****************************************//****************************************

int evaluar_opciones(Entrenador entrenador, Pokenest pokenest)
{
	/*Accion numero 1
	Solicitar al mapa la ubicación de la PokeNest del próximo Pokémon que desea obtener, en caso de aún no conocerla.
	Necesitamos una estructura Pokenest que guarde la ubicación de la misma
	*/

	if(faltaPokenest(pokenest))
	{
		return 1; //Tengo que pedirle la Pokenest al server
	}
	if(llegueAPokenest(entrenador,pokenest))
	{
		return 3; //Tengo que atrapar un Pokemon
	}
	return 2; //Tengo que caminar

	//ToDo falta la opcion de notificar fin de objetivos!
}
//****************************************//****************************************

void recv_solicitarPokenest(Pokenest* pokenest, int fd_server)
{
	int valor_recv;
	int codigo;
	int tam_buffer = size_POKENEST_response;

	char* buffer = malloc(tam_buffer);

	valor_recv = recv(fd_server, buffer, tam_buffer, 0);// Se almacena el mensaje recibido en BUFFER

	if(valor_recv == -1)  {
		perror("Error recv"); //Hubo fallo, chau programa.
		exit(1);
	}

	//Verificamos que el codigo sea correcto
	memcpy(&codigo,buffer,sizeof(int));

	if(codigo != POKENEST)
	{
		perror("CODIGO DE ERROR - RECV POKENEST");
		exit(1);
	}

	char simboloAux;

	memcpy(&simboloAux,buffer+sizeof(int),sizeof(char));

	if(pokenest->simbolo != simboloAux)
	{
		exit(1);
	}

	memcpy(&(pokenest->posx),buffer+sizeof(int)+sizeof(char),sizeof(int));
	memcpy(&(pokenest->posy),buffer+sizeof(int)+sizeof(char)+sizeof(int),sizeof(int));

	free(buffer);
}

void calcular_coordenadas(Entrenador* entrenador, int x, int y)
{
	//Pregunto por X

	if(x > entrenador->posx)
	{
		entrenador ->destinox = x - entrenador->posx;
	}

	if(entrenador->posx == x)
	{
		entrenador ->destinox = 0;
	}

	if(x < entrenador->posx)
	{
		entrenador->destinox = x - entrenador->posx;
	}

	//Pregunto por y

	if(y > entrenador->posy)
	{
		entrenador ->destinoy = y - entrenador->posy;
	}

	if(entrenador->posy == y)
	{
		entrenador ->destinoy = 0;
	}

	if(y < entrenador->posy)
	{
		entrenador->destinoy = y - entrenador->posy;
	}
}




//*********************************
//********************************

void send_coordenadas(Entrenador entrenador)
{

}


Paquete srlz_solicitarPokenest(Pokenest pokenest)
{
	Paquete paquete;
	paquete.buffer = malloc(size_POKENEST_request);
	paquete.tam_buffer = size_POKENEST_request;

	int codigo = POKENEST;

	//Copiamos primero el codigo, despues el simbolo de la Pokenest
	memcpy(paquete.buffer,&codigo,sizeof(int));
	memcpy(paquete.buffer+sizeof(int),&(pokenest.simbolo),sizeof(char));

	return paquete;
}

//************************************************************************

void manejar_signals(int operacion){
	switch(operacion){
	case SIGUSR1:
		vidas_restantes = vidas_restantes + 1;
		break;
	case SIGTERM:
		vidas_restantes = vidas_restantes - 1;
		if(vidas_restantes == 0){
			exit(1);
		}
		break;
	}
}


void sigHandler_endProcess(int signal)
{
	switch(signal)
	{
	case SIGINT || SIGHUP:
		close(fd_server);
		printf("Atrapando %i ", signal);
		exit(1);
		break;

	}
}

//*****************************************************************************

Paquete recv_capturarPokemon(int fd_server)
{
	Paquete paquete;
	paquete.tam_buffer = sizeof(int)*2+sizeof(char)*200;
	paquete.buffer = malloc(paquete.tam_buffer);


	recv(fd_server,paquete.buffer,paquete.tam_buffer,0);

	return paquete;
}

int dsrlz_codigoOperacion(void* buffer)
{
	int codOp;
	memcpy(&codOp,buffer,sizeof(int));
	return codOp;
}

char* dsrlz_capturarPokemon(Paquete* paquete)
{
	int codOp;
	int lengthPokemonDat;
	char *pokemonDat;

	codOp = dsrlz_codigoOperacion(paquete->buffer);

	if(codOp != CAPTURA_OK)
	{
		perror("dsrlz_capturarPokemon - Codigo de Operacion CAPTURA_OK invalido");
		exit(1);
	}

	memcpy(&lengthPokemonDat,paquete->buffer+sizeof(int),sizeof(int));

	pokemonDat = malloc(sizeof(char)*lengthPokemonDat+1);

	memcpy(pokemonDat,paquete->buffer+sizeof(int)*2,sizeof(char)*lengthPokemonDat);

	return pokemonDat;
}

int sizeofString(char* cadena)
{
	int size = 0;
	size = sizeof(char)*strlen(cadena);
	return size;
}
char* getRutaMetadata(ParametrosConsola parametros)
{
	char* pathEntrenadores= "Entrenadores";
	char* pathMetadata = "metadata.config";
	char* slash = "/";

	int tamPathPokedex =sizeofString(parametros.dirPokedex);
	int tamNombreEntrenador = sizeofString(parametros.nombreEntrenador);
	int tamPathEntrenadores = sizeofString(pathEntrenadores);
	int tamSlash = sizeofString(slash);
	int tamPathMetadata = sizeofString(pathMetadata);

	// RUTAPOKEDEX + SLASH + ENTRENADORES + SLASH + NOMBREENTRENADOR + SLASH + METADATA
	int tamCadena = tamPathPokedex + tamSlash + tamPathEntrenadores + tamSlash + tamNombreEntrenador + tamSlash + tamPathMetadata + 1;

	char* rutaALeer = malloc(tamCadena);

	if(rutaALeer == NULL)
	{
		perror("funcion: rutaMetadata() - rutaALeer == NULL - Error Malloc()" );
		exit(1);
	}

	strcpy(rutaALeer,parametros.dirPokedex);
	strcat(rutaALeer,slash);
	strcat(rutaALeer,pathEntrenadores);
	strcat(rutaALeer,slash);
	strcat(rutaALeer,parametros.nombreEntrenador);
	strcat(rutaALeer,slash);
	strcat(rutaALeer,pathMetadata);


	return rutaALeer;



}

metadata leerMetadataEntrenador(ParametrosConsola parametros)
{
	metadata mdata;
	char* rutaMetadata = getRutaMetadata(parametros);
	mdata = leerMetadata(rutaMetadata);

	free(rutaMetadata);
	return mdata;

}

int dsrlz_MoverOK(void* buffer)
{
	int codOp = 0;
	memcpy(&codOp,buffer,sizeof(int));
	return codOp;
}


void recv_MoverOK(int fdServer)
{
	Paquete paquete;
	paquete.tam_buffer = size_MOVER_OK;
	paquete.buffer = malloc(paquete.tam_buffer);

	recv(fdServer,paquete.buffer,paquete.tam_buffer,0);

	int codigoOperacion = dsrlz_MoverOK(paquete.buffer);

	if(codigoOperacion!=MOVER_OK)
	{
		printf("Exit(1) - Func %s - Linea: %d - Codigo MOVER_OK invalido",__func__,__LINE__);
		exit(1);
	}

}

int main(int argc, char** argv)
{
	ParametrosConsola parametros;
	/*Recibimos el nombre del entrenador y la direccion de la pokedex por Consola*/

	/*
	verificarParametros(argc); //Verificamos que la cantidad de Parametros sea correcta
	parametros = leerParametrosConsola(argv); //Leemos los parametros necesarios
	*/

	parametros.dirPokedex = "/mnt/pokedex";
	parametros.nombreEntrenador = "Ash";

	//Ahora se deberia leer la Hoja de Viaje, la direccion de la Pokedex esta en parametros.dirPokedex

	metadata mdata;
	mdata = leerMetadataEntrenador(parametros);


	//A partir de aca, comienza el juego, es decir hacer acciones en el mapa

	Nivel nivel = new_nivel();
	DatosMapa mapa;

	int opcion = -1;
	Entrenador entrenador;
	entrenador = new_Entrenador(mdata);
	vidas_restantes = entrenador.vidas;
	Paquete paquete;

	//Agregamos las funciones que manejaran las señales enmascaras como SIGTERM Y SIGUSR1.
	signal(SIGUSR1, manejar_signals);
	signal(SIGTERM, manejar_signals);

	signal(SIGINT,sigHandler_endProcess);
	signal(SIGHUP,sigHandler_endProcess);

	//A partir de aca, comienza el juego, es decir hacer acciones en el mapa

	Pokenest pokenest;
	char* pokemonDat;
	char pokenestSiguiente;

	while(1)
	{
		mapa.nombre = obtenerNombreMapa(mdata.hojaDeViaje,nivel.nivelActual); //Obtenemos el nombre del mapa numero X

		ConexionEntrenador connect;
		connect = leerConexionMapa(parametros.dirPokedex,mapa.nombre); //Leemos la información del Mapa nombre "LoQueSea"

		//Ahora debemos conectarnos al mapa
		fd_server = get_fdServer(connect.ip,connect.puerto);
		send_simboloEntrenador(mdata.simbolo, fd_server);

		/*A partir de aca, ya nos conectamos al mapa, asi que tenemos que estar dentro de un While Interno hasta que terminamos
		 * de capturar todos los pokemon, cuando terminamos, salimos del while interno y volvemos al externo.
		 */
		nivel.finNivel = 0;
		pokenest = new_pokenest(mdata.objetivos[nivel.nivelActual],nivel.numPokenest);
		while(nivel.finNivel == 0) //Mientras que no hayamos ganado el nivel
		{

				opcion = evaluar_opciones(entrenador,pokenest);

				switch(opcion)
				{
					case POKENEST://Caso 1: QUEREMOS UNA POKENEST!

						paquete = srlz_solicitarPokenest(pokenest);
						send_solicitarPokenest(&paquete,fd_server);
						recv_solicitarPokenest(&pokenest, fd_server);
						calcular_coordenadas(&entrenador,pokenest.posx,pokenest.posy);
						//send_coordenadas(entrenador);
					break;
					case MOVER://Caso 2: Queremos movernos!
						mover_entrenador(&entrenador);
						paquete = srlz_movEntrenador(entrenador);
						send_movEntrenador(&paquete,fd_server);
						recv_MoverOK(fd_server);
					break;
					case CAPTURAR: //Caso 3: Ya llegamos a una Pokenest. QUEREMOS CAPTURAR
						paquete = srlz_capturarPokemon(pokenest.simbolo);
						send_capturarPokemon(&paquete,fd_server);
						paquete = recv_capturarPokemon(fd_server);
						pokemonDat = dsrlz_capturarPokemon(&paquete);
						//fflush(stdout);
						printf("%s - Objetivo Numero: %i \n",pokemonDat,nivel.numPokenest);
						nivel.numPokenest++;


						if(getNumObjetivo(nivel.numPokenest,mdata.hojaDeViaje[nivel.nivelActual]) == '\0')
						{
							printf("Fin Nivel\n");
							close(fd_server);
							nivel.finNivel = 1;
							nivel.nivelActual++;
							nivel.numPokenest = 0;
						}
						else
						{
							pokenest = new_pokenest(mdata.objetivos[nivel.nivelActual],nivel.numPokenest);
						}
					break;
					case FINOBJETIVOS:
						paquete = srlz_finObjetivos();
						send_finObjetivos(&paquete,fd_server);
					break;
				}

				//free(paquete.buffer);
		//FUERA DEL WHILE INTERNO
		//Una vez que salimos del While interno, quiere decir que terminamos el mapa y hay que pasar a otro
		//Antes, el entrenador tiene que dirigirse a la Pokedex y copiar la respectiva medalla del mapa

		//Ahora si pasamos al siguiente mapa, podemos hacer numeroMapaActual++ y repetimos.
		//En caso de que hayamos llegado al limite de mapas, hay que salir del while porque terminamos el juego.
	}
	}
	//free(paquete.buffer);
	return 0;
}
