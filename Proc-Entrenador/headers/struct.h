/*
 * struct.h
 *
 *  Created on: 30/8/2016
 *      Author: utnso
 */

#ifndef HEADERS_STRUCT_H_
#define HEADERS_STRUCT_H_

typedef struct //metadata
{
	char* nombre;
	char simbolo;
	char** hojaDeViaje;
	char** objetivos;
	int vidas;
	int reintentos;
}metadata;
//**********************************************************************************************************

typedef struct //ParametrosConsola
{
	char* nombreEntrenador;
	char* dirPokedex;
}ParametrosConsola;
//**********************************************************************************************************

typedef struct //conexionEntrenador
{
	char* ip;
	char* puerto;
}ConexionEntrenador;
//**********************************************************************************************************

typedef struct{ //Entrenador
	char simbolo;
	int posx;
	int posy;
	char movAnterior;
	int flagx;
	int flagy;
	int destinox;
	int destinoy;
	int vidas;
	int reintentos;
	char* nombre;
}Entrenador;
//**********************************************************************************************************

typedef struct //Pokenest
{
	int posx;
	int posy;
	char simbolo;
}Pokenest;
//**********************************************************************************************************

typedef struct //Paquete
{
	void* buffer;
	int tam_buffer;
}Paquete;
//**********************************************************************************************************

typedef struct //DatosMapa
{
	char* nombre;
}DatosMapa;
//**********************************************************************************************************

typedef struct //Nivel
{
	int finNivel;
	int nivelActual;
	int numPokenest;
	int cantObjetivos;
	int cantNiveles;
}Nivel;

//***************//********!!!!!!!!!!!NEW DE STRUCTS!!!!!!!!!!!!!!//***************//***************

Pokenest new_pokenest(char** objetivos, int num)
{
	Pokenest pokenest;
	pokenest.posx = -1;
	pokenest.posy = -1;
	char* aux = strdup(objetivos[num]);
	memcpy(&(pokenest.simbolo),aux, sizeof(char));//TODO arreglar memory leak
	return pokenest;
}
//**********************************************************************************************************

DatosMapa new_DatosMapa()
{
	DatosMapa mapa;
	return mapa;
}
//**********************************************************************************************************

Nivel new_nivel()
{
	Nivel nivel;
	nivel.finNivel=0;
	nivel.nivelActual=0;
	nivel.numPokenest=0;
	return nivel;
}
//**********************************************************************************************************

Entrenador new_Entrenador(metadata mdata)
{
	Entrenador entrenador;
    entrenador.posx = 1;
    entrenador.posy = 1;
    entrenador.simbolo = mdata.simbolo;
    entrenador.movAnterior = 'y';
    entrenador.flagx = 0;
    entrenador.flagy = 0;
    entrenador.nombre = strdup(mdata.nombre);
    entrenador.vidas = mdata.vidas;
    entrenador.reintentos = mdata.reintentos;
    return entrenador;
}
//********************
//*** FUNCIONES BOLUDAS
//*************

int sizeofString(char* cadena)
{
	int size = 0;
	size = sizeof(char)*strlen(cadena);
	return size;
}

#endif /* HEADERS_STRUCT_H_ */
