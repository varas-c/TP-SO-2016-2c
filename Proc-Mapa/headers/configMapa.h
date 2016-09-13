/*
 * config.h
 *
 *  Created on: 13/9/2016
 *      Author: utnso
 */

#ifndef HEADERS_CONFIGMAPA_H_
#define HEADERS_CONFIGMAPA_H_

/****************************************************************************************************************
 * ************************************************************************************************************
 *
			FUNCIONES DE LECTURA DE ARCHIVOS

****************************************************************************************************************
****************************************************************************************************************/



/* leerMetadataMapa:
 * Lee todos los campos de un archivo Metadata Mapa y los guarda en un struct
 */

MetadataMapa leerMetadataMapa()
{
	MetadataMapa mdata;
	t_config* config; //Estructura
	char* auxiliar;

	//RUTA ABSOLUTA
	//config = config_create("//home/utnso/SistOp/tp-2016-2c-Breaking-Bug/Proc-Mapa/config/mapa.config");
	//RUTA RELATIVA
	config = config_create("../config/mapa.config");

	if(config==0)
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

/* leerMetadaPokenest:
 * Lee todos los campos de un archivo Metadata Pokenest y los guarda en un struct
 */

MetadataPokenest leerMetadataPokenest()
{
	MetadataPokenest mdata;
	t_config* config; //Estructura

	char* auxiliar;

	//RUTA ABSOLUTA
	//config = config_create("/home/utnso/SistOp/tp-2016-2c-Breaking-Bug/Proc-Mapa/config/pokenest.config");
	//RUTA RELATIVA
	config = config_create("../config/pokenest.config");

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

/* leerMetadaPokemon:
 * Lee todos los campos de un archivo Metadata Pokemon y los guarda en un struct
 */


MetadataPokemon leerMetadataPokemon()
{
	MetadataPokemon mdata;
	t_config* config; //Estructura

	//RUTA ABSOLUTA
	//config = config_create("/home/utnso/SistOp/tp-2016-2c-Breaking-Bug/Proc-Mapa/config/pokemon.config");
	//RUTA RELATIVA
	config = config_create("../config/pokemon.config");

	if(config==0)
	{
		printf("Archivo pokemon.config no encontrado\n");
		exit(20);
	}

	mdata.nivel = config_get_int_value(config, "Nivel");

	config_destroy(config);

	return mdata;
}

#endif /* HEADERS_CONFIGMAPA_H_ */
