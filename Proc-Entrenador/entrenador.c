/*
 * entrenador.c
 *
 *  Created on: 29/8/2016
 *      Author: utnso
 */
#include <stdio.h>
#include <string.h>
#include <commons/config.h>

typedef struct
{
	char* nombre;
	char simbolo;
	char** hojaDeViaje;
	char** objetivos;
	int vidas;
	int reintentos;

}metadata;


typedef struct
{
	char* nombreEntrenador;
	char* dirPokedex;
}ParametrosConsola;

ParametrosConsola leerParametrosConsola(char** parametros)
{
	ParametrosConsola p;

	p.nombreEntrenador = malloc(strlen(parametros[1])+1);
	p.dirPokedex = malloc(strlen(parametros[2])+1);
	strcat(p.nombreEntrenador,parametros[1]);
	strcat(p.dirPokedex,parametros[2]);

	return p;
}

void destruct_ParametrosConsola(ParametrosConsola *p)
{
	free(p->nombreEntrenador);
	free(p->dirPokedex);
}


void verificarParametros(int argc)
{
	if(argc!=3)
	{
		printf("Error - Faltan parametros");
		exit(1);
	}
}



//El calculo es 4 de "obj[" + largociudad + "]" + /n
char* concatObjetivo(char* ciudad, char* obj)
{
	obj = malloc(4*sizeof(char)+strlen(ciudad)*sizeof(char)+2);

	strcat(obj,"obj[");
	strcat(obj,ciudad);
	strcat(obj,"]");

	return obj;

}

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


metadata leerMetadata()
{
	metadata mdata;
	t_config* config; //Estructura
	int cantViajes;

	config = config_create("config/metadata.config"); //Se lee el archivo con config_create y se almacena lo leido en config

	mdata.nombre = config_get_string_value(config, "nombre");
	mdata.simbolo = config_get_string_value(config, "simbolo");
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
	mdata.reintentos = config_get_int_value(config,"vidas");

	return mdata;
}



int main(int argc, char** argv)
{
	/*
	//Recibimos el nombre del entrenador y la direccion de la pokedex por Consola
	 *
	ParametrosConsola parametros;
	verificarParametros(argc); //Verificamos que la cantidad de Parametros sea correcta
	parametros = leerParametrosConsola(argv); //Leemos los parametros necesarios
	printf("%s --- %s", parametros.dirPokedex, parametros.nombreEntrenador);
	destruct_ParametrosConsola(&parametros);//Liberamos los parametros

	*/

	//--------------------------------
	//Ahora se deberia leer la Hoja de Viaje, la direccion de la Pokedex esta en parametros.dirPokedex
	metadata mdata;
	mdata = leerMetadata();


	//-------------
	//Leida la hoja de viaje, se debe buscar el socket y puerto del primer mapa




	return 0;
}

