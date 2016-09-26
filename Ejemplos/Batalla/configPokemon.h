/*
 * config.h
 *
 *  Created on: 13/9/2016
 *      Author: utnso
 */

#ifndef HEADERS_CONFIGPOKEMON_H_
#define HEADERS_CONFIGPOKEMON_H_

/****************************************************
			FUNCIONES DE LECTURA DE ARCHIVOS
****************************************************/

//****************************************************************************************************************

MetadataPokemon leerMetadataPokemon(char* ruta)   //Lee todos los campos de un archivo Metadata Pokemon y los guarda en un struct
{
	MetadataPokemon mdata;
	t_config* config; //Estructura

	//RUTA ABSOLUTA
	//config = config_create("/home/utnso/SistOp/tp-2016-2c-Breaking-Bug/Proc-Mapa/config/pokemon.config");
	//RUTA RELATIVA
	config = config_create(ruta);

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


#endif /* HEADERS_CONFIGPOKEMON_H_ */
