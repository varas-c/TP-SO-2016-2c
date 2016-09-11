/*
 * entrenador.c
 *
 *  Created on: 29/8/2016
 *      Author: utnso
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <commons/config.h>
#include "headers/struct.h"
#include "headers/socket.h"
#include <sys/ioctl.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <commons/log.h>

enum codigoOperaciones {
	TURNO = 0,
	POKENEST = 1,
};

enum sizeofBuffer
{
	size_TURNO = sizeof(int),
	size_POKENEST = sizeof(int) + sizeof(char)+sizeof(int)+sizeof(int)
};



//Lee los parametros por consola y los guarda en un struct

ParametrosConsola leerParametrosConsola(char** parametros)
{
	ParametrosConsola p;

	p.nombreEntrenador = malloc(strlen(parametros[1])+1); //Reservamos memoria
	p.dirPokedex = malloc(strlen(parametros[2])+1);

	strcat(p.nombreEntrenador,parametros[1]); //guardamos
	strcat(p.dirPokedex,parametros[2]);

	return p;
}

//******************************************

//Destructor de un struct ParametrosConsola
void destruct_ParametrosConsola(ParametrosConsola *p)
{
	free(p->nombreEntrenador);
	free(p->dirPokedex);
}

//******************************************
//Verifica que los parametros pasados por consola sean los necesarios, solo los cuenta, nada mas.
void verificarParametros(int argc)
{
	if(argc!=3)
	{
		printf("\n\n Error - Faltan parametros\n\n");
		exit(1);
	}
}


//******************************************
//El calculo es 4 de "obj[" + largociudad + "]" + /n
char* concatObjetivo(char* ciudad, char* obj)
{
	obj = malloc(4*sizeof(char)+strlen(ciudad)*sizeof(char)+2);

	strcat(obj,"obj[");
	strcat(obj,ciudad);
	strcat(obj,"]");

	return obj;

}

//******************************************
//Muestra los objetivos de un mapa
void mostrarObjetivos (char **a)
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

//******************************************
//Cuenta la cantidad de viajes en una hoja de Viaje
int cantidadDeViajes(char** hojaDeViaje)
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

//Lee Metadata de un entrenador
metadata leerMetadata()
{
	metadata mdata;
	t_config* config; //Estructura config
	int cantViajes;
	char* auxiliar;

	//Se lee el archivo con config_create y se almacena lo leido en config
	config = config_create("../config/metadata.config");

	//Si no se pudo abrir el archivo, salimos
	if (config==NULL)
	{
		printf("Archivo metadata.config no encontrado.\n");
		exit(20);
	}


	//Leemos el nombre en un auxiliar, luego lo copiamos
	auxiliar = config_get_string_value(config,"nombre");
	mdata.nombre = malloc(strlen(auxiliar)+1);
	strcpy(mdata.nombre, auxiliar);

	//auxiliar = config_get_string_value(config,"simbolo");
	mdata.simbolo = *config_get_string_value(config,"simbolo");
	//strcpy(mdata.simbolo, auxiliar);

	mdata.hojaDeViaje = config_get_array_value(config, "hojaDeViaje");


	//---------------ACA SE LEEN LOS OBJETIVOS -------------
	cantViajes = cantidadDeViajes(mdata.hojaDeViaje);
	mdata.objetivos = malloc(cantViajes*sizeof(char*));

	//Esto deberia ir en una funcion aparte pero no se como pasar por parametro mdata.objetivos porque es un char**
	//Hay una funcion leerObjetivos que habría que arreglar
	////leerObjetivos(mdata.objetivos, config,cantViajes,mdata.hojaDeViaje);

	char* ciudad;
	char* leer = NULL;

	int i;
	for(i=0;i<cantViajes;i++)
	{
		//4*sizeof(char)+strlen(ciudad)*sizeof(char)+2
		ciudad = mdata.hojaDeViaje[i];
		leer = malloc(4*sizeof(char)+strlen(ciudad)*sizeof(char)+2);
		strcpy(leer,"obj[");
		strcat(leer,ciudad);
		strcat(leer,"]");
		mdata.objetivos[i] = config_get_array_value(config,leer);
		mostrarObjetivos(mdata.objetivos[i]);
		free(leer);
	}

	//---------------------------------------------------------

	mdata.vidas = config_get_int_value(config,"vidas");
	mdata.reintentos = config_get_int_value(config,"reintentos");

	config_destroy(config);
	return mdata;
}

ConexionEntrenador leerConexionMapa(int mapa)
{
	ConexionEntrenador connect;
	t_config* config; //Estructura
	char* auxiliar;


	//****************************
	//ToDo: Aca debemos leer el mapa.config del mapa que recibimos por parametro
	config = config_create("/home/utnso/tp-2016-2c-Breaking-Bug/Proc-Mapa/config/mapa.config");
	//config = config_create("../../Proc-Mapa/config/mapa.config");

	if(config==NULL)
	{
		printf("Archivo mapa.config no encontrado\n");
		exit(20);
	}

	//Leemos el puerto
	auxiliar = config_get_string_value(config,"Puerto");
	connect.puerto = malloc(strlen(auxiliar)+1);
	strcpy(connect.puerto, auxiliar);

	//Leemos la IP
	auxiliar = config_get_string_value(config,"IP");
	connect.ip = malloc(strlen(auxiliar)+1);
	strcpy(connect.ip, auxiliar);

	//Cerramos el archivo
	config_destroy(config);
	return connect;

}



Pokenest new_pokenest(char** objetivos, int num)
{
	Pokenest pokenest;
	pokenest.posx = -1;
	pokenest.posy = -1;
	char* aux = strdup(objetivos[num]);
	memcpy(&pokenest.simbolo,aux, sizeof(char));
	return pokenest;
}

typedef struct
{
	char* nombre;
}DatosMapa;

DatosMapa new_DatosMapa()
{
	DatosMapa mapa;
	return mapa;
}

//Si flagx=false then mover en X otherwise mover en Y
void mover_entrenador(Entrenador *entrenador)
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



char* obtenerNombreMapa(char** hojaDeViaje, int numeroMapa)
{
	char *a = strdup(hojaDeViaje[numeroMapa]);
	return a;
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

//Si la pokenest tiene posx y posy = -1 es porque no tenemos datos, asi que hay que leer
int faltaPokenest(Pokenest pokenest)
{
	return pokenest.posx == -1 || pokenest.posy == -1;
}

//****************************************//****************************************
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
//****************************************//****************************************

void send_solicitarPokenest(Pokenest *pokenest, int fd_server)
{
	void* buffer;
	int tam_buffer = sizeof(int)+sizeof(char);
	buffer = malloc(tam_buffer);

	int codigo = POKENEST;

	//Copiamos primero el codigo, despues el simbolo de la Pokenest
	memcpy(buffer,&codigo,sizeof(int));
	memcpy(buffer+sizeof(int),&(pokenest->simbolo),sizeof(char));

	send(fd_server, buffer, tam_buffer,0);

	free(buffer);
}

//****************************************//****************************************
//****************************************//****************************************



void send_moverEntrenador(Entrenador *entrenador)
{
	//Moverme
	/*Accion numero 2:
	* Avanzará una posición hacia la siguiente PokeNest1,
	* * informando el movimiento al Mapa, en caso de aún no haber llegado a ella.
	*
	* SI ya conocemos la ubicacion de la Pokenest, hacemos el movimiento correspondiente,
	* actualizamos NUESTRA estructura entrenador con la nueva coordenada y le debemos indicar
	* al server QUE coordenada modificamos y el nuevo valor de dicha coordenada
	*
	El mensaje sería "---- Coordenada ---- Valor-----"
	*Ejemplo: "X25", "Y31", "X8", etc
	 */
}

//****************************************//****************************************
//****************************************//****************************************

void send_capturarPokemon()
{

}
//****************************************//****************************************
//****************************************//****************************************

void send_finObjetivos()
{

}

//****************************************//****************************************
//****************************************//****************************************

typedef struct
{
	int finNivel;
	int nivelActual;
	int numPokenest;
}Nivel;

Nivel new_nivel()
{
	Nivel nivel;
	nivel.finNivel=0;
	nivel.nivelActual=0;
	nivel.numPokenest=0;

	return nivel;
}

void recv_solicitarPokenest(Pokenest* pokenest, int fd_server)
{

	int valor_recv;
	int codigo;
	int tam_buffer = size_POKENEST;

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

}


int main(int argc, char** argv)
{

	/*
	//Recibimos el nombre del entrenador y la direccion de la pokedex por Consola


	ParametrosConsola parametros;
	verificarParametros(argc); //Verificamos que la cantidad de Parametros sea correcta
	parametros = leerParametrosConsola(argv); //Leemos los parametros necesarios
	printf("%s --- %s", parametros.dirPokedex, parametros.nombreEntrenador);
	destruct_ParametrosConsola(&parametros);//Liberamos los parametros

	//--------------------------------
	//Ahora se deberia leer la Hoja de Viaje, la direccion de la Pokedex esta en parametros.dirPokedex
	*/

	metadata mdata;
	mdata = leerMetadata();
	//--------------------------------------------
	//----------------------------------------------

	//A partir de aca, comienza el juego, es decir hacer acciones en el mapa

	//--------------------------------------------
	//----------------------------------------------


	Nivel nivel = new_nivel();
	DatosMapa mapa = new_DatosMapa();

	Pokenest pokenest = new_pokenest(mdata.objetivos[nivel.nivelActual],nivel.numPokenest);

	int opcion = -1;
	Entrenador entrenador;



	/* Este while es un "While Externo", el While externo es para poder avanzar de mapa una vez finalizado
	 * el mapa actual
	 */


	//A partir de aca, comienza el juego, es decir hacer acciones en el mapa

	while(1)
	{
		mapa.nombre = obtenerNombreMapa(mdata.hojaDeViaje,nivel.nivelActual); //Obtenemos el nombre del mapa numero X

		ConexionEntrenador connect;
		connect = leerConexionMapa(mapa.nombre); //Leemos la información del Mapa nombre "LoQueSea"


		//Ahora debemos conectarnos al mapa
		int fd_server = get_fdServer(connect.ip,connect.puerto);

		/*A partir de aca, ya nos conectamos al mapa, asi que tenemos que estar dentro de un While Interno hasta que terminamos
		 * de capturar todos los pokemon, cuando terminamos, salimos del while interno y volvemos al externo.
		 */

		while(nivel.finNivel == 0) //Mientras que no hayamos ganado el nivel
		{

			if(recv_turnoConcedido(fd_server))
			{
				opcion = evaluar_opciones(entrenador,pokenest);

				switch(opcion)
				{
					case 1:
						send_solicitarPokenest(&pokenest, fd_server);
						recv_solicitarPokenest(&pokenest, fd_server);
					break;
					case 2:
						send_moverEntrenador(&entrenador);
					break;
					case 3:
						send_capturarPokemon();
					break;
					case 4:
						send_finObjetivos();
					break;
				}


				/*ACCION NUMERO 3:
				 * Al llegar a las coordenadas de una PokeNest, solicitará al mapa atrapar un Pokémon.
				* El Entrenador quedará bloqueado hasta que el Mapa le confirme la captura del Pokémon.
				* En ese momento, el Entrenador evaluará (e informará al Mapa) si debe continuar atrapando Pokémons en este Mapa,
					* o si ya cumplió sus objetivos en el mismo.

				* Para hacerlo tenemos el struct Entrenador y la Pokenest, basicamente es comparar si las coordenadas son iguales
					 *Para saber si ya cumplimos los objetivos, se puede crear una variable "cantidadDePokemonAAtrapar" que cuente cuantos Pokemon
					*tenemos que atrapar en este mapa. A medida que vamos capturando, descontamos. Si es es 0, terminamos el nivel, le informamos al mapa
					*salimos del mismo con un close connection
					*
					*/
			}

		}

		//FUERA DEL WHILE INTERNO
		//Una vez que salimos del While interno, quiere decir que terminamos el mapa y hay que pasar a otro
		//Antes, el entrenador tiene que dirigirse a la Pokedex y copiar la respectiva medalla del mapa

		//Ahora si pasamos al siguiente mapa, podemos hacer numeroMapaActual++ y repetimos.
		//En caso de que hayamos llegado al limite de mapas, hay que salir del while porque terminamos el juego.

	}

	return 0;
}

