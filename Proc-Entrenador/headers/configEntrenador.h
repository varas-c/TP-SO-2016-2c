/*
 * configEntrenador.h
 *
 *  Created on: 13/9/2016
 *      Author: utnso
 */

#ifndef HEADERS_CONFIGENTRENADOR_H_
#define HEADERS_CONFIGENTRENADOR_H_

metadata leerMetadata(char* ruta)//Lee Metadata de un entrenador
{
	metadata mdata;
	t_config* config; //Estructura config
	int cantViajes;
	char* auxiliar;

	//Se lee el archivo con config_create y se almacena lo leido en config
	config = config_create(ruta);

	//Si no se pudo abrir el archivo, salimos
	if (config==NULL)
	{
		printf("Error en - Funcion: %s - Linea: %d - Archivo metadata.config no encontrado.\n", __func__,__LINE__);
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
//******************************************

char* getRutaMapa(char* pathPokedex, char* nombreMapa)
{
	char *ruta;

	char* Mapas = "/Mapas/";
	char* Metadata = "/metadata";
	int tamPathPokedex = sizeofString(pathPokedex);
	int tamNombreMapa = sizeofString(nombreMapa);
	int tamMapas = sizeofString(Mapas);
	int tamMetadata = sizeofString(Metadata);


	int tamCadena = tamPathPokedex + tamMapas + tamNombreMapa + tamMetadata + 1;

	ruta = malloc(tamCadena);

	if(ruta==NULL)
	{
		printf("Error - Funcion: %s - Linea: %d - malloc == NULL",__func__,__LINE__);
		exit(1);
	}

	sprintf(ruta,"%s%s%s%s",pathPokedex,Mapas,nombreMapa,Metadata);

	return ruta;
}
//******************************************

ConexionEntrenador leerConexionMapa(char* pathPokedex, char* nombreMapa)
{
	ConexionEntrenador connect;
	t_config* config; //Estructura
	char* auxiliar;
	char* rutaMapa;
	rutaMapa = getRutaMapa(pathPokedex, nombreMapa);
	//ToDo: Aca debemos leer el mapa.config del mapa que recibimos por parametro
	//config = config_create("/home/utnso/tp-2016-2c-Breaking-Bug/Proc-Mapa/config/mapa.config");
	//config = config_create("../../Proc-Mapa/config/mapa.config");
	config = config_create(rutaMapa);
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
//******************************************

ParametrosConsola leerParametrosConsola(char** parametros)  //Lee los parametros por consola
{															//y los guarda en un struct
	ParametrosConsola p;

	p.dirPokedex = strdup(parametros[1]);
	p.nombreEntrenador = strdup(parametros[2]);

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
//******************************************

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
//******************************************

metadata leerMetadataEntrenador(ParametrosConsola parametros)
{
	metadata mdata;
	char* rutaMetadata = getRutaMetadata(parametros);
	mdata = leerMetadata(rutaMetadata);

	free(rutaMetadata);
	return mdata;

}
//******************************************

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
//******************************************

int getCantObjetivos(char** objetivos)
{
	int cantObjetivos=-1;
	int index = 0;

	while(objetivos[index] != '\0')
	{
			cantObjetivos++;
			index++;
	}

	return cantObjetivos;


}

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

Paquete recv_capturarPokemon(int fd_server)
{
	Paquete paquete;
	paquete.tam_buffer = sizeof(int)*2+sizeof(char)*50+sizeof(int)+sizeof(char)*50+sizeof(t_pokemon);
	paquete.buffer = malloc(paquete.tam_buffer);


	recv(fd_server,paquete.buffer,paquete.tam_buffer,0);

	return paquete;
}



#endif /* HEADERS_CONFIGENTRENADOR_H_ */
