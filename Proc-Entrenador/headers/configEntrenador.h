/*
 * configEntrenador.h
 *
 *  Created on: 13/9/2016
 *      Author: utnso
 */

#ifndef HEADERS_CONFIGENTRENADOR_H_
#define HEADERS_CONFIGENTRENADOR_H_

metadata leerMetadata()//Lee Metadata de un entrenador
{
	metadata mdata;
	t_config* config; //Estructura config
	int cantViajes;
	char* auxiliar;

	//Se lee el archivo con config_create y se almacena lo leido en config
	config = config_create("config/metadata.config");

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

	//ToDo: Aca debemos leer el mapa.config del mapa que recibimos por parametro
	//config = config_create("/home/utnso/tp-2016-2c-Breaking-Bug/Proc-Mapa/config/mapa.config");
	//config = config_create("../../Proc-Mapa/config/mapa.config");
	config = config_create("/home/utnso/TPChar*Mander/tp-2016-2c-Breaking-Bug/Proc-Mapa/config/mapa.config");
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

ParametrosConsola leerParametrosConsola(char** parametros)  //Lee los parametros por consola
{															//y los guarda en un struct
	ParametrosConsola p;

	p.nombreEntrenador = malloc(strlen(parametros[1])+1); //Reservamos memoria
	p.dirPokedex = malloc(strlen(parametros[2])+1);

	strcat(p.nombreEntrenador,parametros[1]); //guardamos
	strcat(p.dirPokedex,parametros[2]);

	return p;
}
//******************************************

void destruct_ParametrosConsola(ParametrosConsola *p) //Destructor de un struct ParametrosConsola
{
	free(p->nombreEntrenador);
	free(p->dirPokedex);
}
//******************************************

void verificarParametros(int argc)//Verifica que los parametros pasados por consola sean los necesarios,
{								  // solo los cuenta, nada mas.
	if(argc!=3)
	{
		printf("\n\n Error - Faltan parametros\n\n");
		exit(1);
	}
}
//******************************************

char* obtenerNombreMapa(char** hojaDeViaje, int numeroMapa)
{
	char *a = strdup(hojaDeViaje[numeroMapa]);
	return a;
}

#endif /* HEADERS_CONFIGENTRENADOR_H_ */
