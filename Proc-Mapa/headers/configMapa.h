/*
 * config.h
 *
 *  Created on: 13/9/2016
 *      Author: utnso
 */

#ifndef HEADERS_CONFIGMAPA_H_
#define HEADERS_CONFIGMAPA_H_

/****************************************************
			FUNCIONES DE LECTURA DE ARCHIVOS
****************************************************/

int sizeofString(char* cadena)
{
	int size = 0;
	size = sizeof(char)*strlen(cadena);
	return size;
}

char* getRutaMapa(ParametrosMapa parametros)
{
	char *ruta;

	char* Mapas = "/Mapas/";
	char* Metadata = "/metadata";
	int tamPathPokedex = sizeofString(parametros.dirPokedex);
	int tamNombreMapa = sizeofString(parametros.nombreMapa);
	int tamMapas = sizeofString(Mapas);
	int tamMetadata = sizeofString(Metadata);


	int tamCadena = tamPathPokedex + tamMapas + tamNombreMapa + tamMetadata + 1;

	ruta = malloc(tamCadena);

	if(ruta==NULL)
	{
		printf("Error - Funcion: %s - Linea: %d - malloc == NULL",__func__,__LINE__);
		exit(1);
	}

	sprintf(ruta,"%s%s%s%s",parametros.dirPokedex,Mapas,parametros.nombreMapa,Metadata);

	return ruta;
}

MetadataMapa leerMetadataMapa(ParametrosMapa parametros)         //Lee todos los campos de un archivo Metadata Mapa y los guarda en un struct
{
	MetadataMapa mdata;
	t_config* config; //Estructura
	char* auxiliar;

	char* ruta;
	ruta = getRutaMapa(parametros);

	//RUTA ABSOLUTA
	//config = config_create("//home/utnso/SistOp/tp-2016-2c-Breaking-Bug/Proc-Mapa/config/mapa.config");
	//RUTA RELATIVA
	config = config_create(ruta);

	if(config==0)
	{
		printf("Error: Funcion: %s - Linea: %d - metadata no encontrado \n",__func__,__LINE__);
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


	mdata.puerto = config_get_int_value(config,"Puerto");

	config_destroy(config);

	return mdata;
}
//****************************************************************************************************************

MetadataPokenest leerMetadataPokenest(char* ruta, char* nombreArchivo) //Lee todos los campos de un archivo Metadata Pokenest y los guarda en un struct
{
	MetadataPokenest mdata;
	t_config* config; //Estructura

	char* auxiliar;

	char* rutaPosta = NULL;
	rutaPosta = malloc(sizeof(char)*256);

	if(rutaPosta == NULL)
	{
		exit(1);
	}

	strcpy(rutaPosta, ruta);
	strcat(rutaPosta,nombreArchivo);

	config = config_create(rutaPosta);

	if(config==0)
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
	memcpy(&(mdata.simbolo),auxiliar,sizeof(char));

	config_destroy(config);

	return mdata;
}
//****************************************************************************************************************

MetadataPokemon leerMetadataPokemon(char* ruta, char* nombreArchivo)   //Lee todos los campos de un archivo Metadata Pokemon y los guarda en un struct
{
	MetadataPokemon mdata;
	t_config* config; //Estructura

	char* rutaPosta = malloc(sizeof(char)*256);

		strcpy(rutaPosta, ruta);
		strcat(rutaPosta,nombreArchivo);

		config = config_create(rutaPosta);

	if(config==0)
	{
		printf("Archivo pokemon.config no encontrado\n");
		exit(20);
	}

	mdata.nivel = config_get_int_value(config, "Nivel");

	config_destroy(config);

	return mdata;
}
//****************************************************************************************************************

ParametrosMapa leerParametrosConsola(char** argv)
{
	ParametrosMapa parametros;
	parametros.nombreMapa = strdup(argv[2]);
	parametros.dirPokedex = strdup(argv[1]);

	return parametros;
}
//****************************************************************************************************************

void verificarParametros(int argc)
{
	if(argc!=3)
	{
		printf("Error - Faltan parametros \n");
		exit(1);
	}
}

#endif /* HEADERS_CONFIGMAPA_H_ */
