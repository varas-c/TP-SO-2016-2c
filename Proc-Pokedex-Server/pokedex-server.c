/*
 * Pokedex-Server.c
 *
 *  Created on: 30/8/2016
 *      Author: utnso
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
//#include <fuse.h>
#include <pthread.h>
#include <time.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/bitarray.h>
#include <commons/string.h>
#include "headers/struct.h"
#include "headers/osada.h"
#include "headers/socket.h"

//ATENCIÓN: off_t ocupa 4 bytes acá, y 8 en el cliente. Tener en cuenta al hacer sizeof(off_t)
//enum codigo_operacion {GETATTR, READDIR, READ, TRUNCATE, WRITE};

#define BLOQUE_NULO 0xFFFFFFFF
#define DIRECTORIO_NULO 0xFFFF
#define COD_GETATTR 1
#define COD_READDIR 2
#define COD_READ 3
#define COD_TRUNCATE 4
#define COD_WRITE 5
#define COD_CREATE 6
#define COD_MKDIR 7
#define COD_RMDIR 8
#define COD_UNLINK 9
#define COD_RENAME 10

#define PUERTO 10000
/* TAMAÑOS ESTRUCTURAS [BLOQUES]: (F= tamaño filesystem)
 *
 * HEADER = 1 (H)
 * BITMAP = F / 512 (/8/64) (B)
 * TABLA ARCHIVOS = 1024 (TAR)
 * TABLA ASIGACIONES = (F - H - B - TAR) / 16 (TAS)
 * DATOS = (F - H - B - TAR - TAS)
 *
 */

//**** VARIABLES GLOBALES ****
osada_header* header;
t_bitarray bitmap;
osada_file* tablaArchivos;
osada_block_pointer* tablaAsignaciones;
int tamanioAsig;
osada_block* inicioDatos;
int tamanioAdmin;

pthread_mutex_t sem_estructuras = PTHREAD_MUTEX_INITIALIZER;

//**** PROTOTIPOS ****
int encontrarBloqueLibre();
void marcarBloque(osada_block_pointer, uint8_t);
void guardarBloque(osada_block, int);
osada_block_pointer* obtenerPtroBloque(int);
int nombrarArchivo(int, char*);
uint8_t sonNombresIguales(unsigned char*, unsigned char*);
void copiarNombreArchivo(char*, int); //Precondición: destino tiene 18 bytes o más
int buscarIndice(unsigned char*, int);
int obtenerPrimerBloque(int);
int obtenerBloqueSgte(int);
int reservarNuevoBloque(int);
int liberarBloque(int, uint8_t);
int borrarArchivo(int); //Valida además que, si es un directorio, esté vacío
int* obtenerBloques(int, int*);
int obtenerArchivo(char*);
int obtenerDirectorioPadre(char*);
char* obtenerNombreArchivo(char*); //Nombre del archivo según ruta, no según tabla de archivos. Hacer free luego invocar
int truncarArchivo(int, uint64_t);
unsigned char* concatenarBloques(int*, int);
int guardarDesdeBloque(int, void*, int, int);
int actualizar(int, void*, size_t, uint64_t);
int crearArchivo(int, const char*, osada_file_state);
int renombrarArchivo(int, char*);



void marcarBloque(osada_block_pointer bloque, uint8_t valor)
{
	if (valor) //Se realizan operaciones a nivel bit (bitwise) para distinguir bits dentro de los bytes del bitmap
		//bitmap[(bloque)/8]= ((unsigned char)pow(2,(8-(bloque)%8)-1)) | (bitmap[(bloque)/8]);
		bitarray_set_bit(&bitmap, bloque+tamanioAdmin);
	else
		//bitmap[(bloque)/8]= ~((unsigned char)pow(2,(8-(bloque)%8)-1)) & (bitmap[(bloque)/8]);
		bitarray_clean_bit(&bitmap, bloque+tamanioAdmin);
}

void guardarBloque(osada_block dato, int bloque) //Podría agregarse la búsqueda de un bloque libre
{
	memset(inicioDatos[bloque], 0, OSADA_BLOCK_SIZE);
	memcpy(inicioDatos[bloque], dato, OSADA_BLOCK_SIZE);
	marcarBloque(bloque, 1);
}

osada_block_pointer* obtenerPtroBloque(int indice)
{
	return inicioDatos+indice;
}

int nombrarArchivo(int indice, char* nombre)
{
	int longNombre = strlen(nombre);

	if (longNombre>17 || indice<0)
		return 0;
	if (longNombre<17)
		longNombre++;
	memcpy(tablaArchivos[indice].fname, nombre, longNombre);
	return 1;
}

uint8_t sonNombresIguales(unsigned char* nombre1, unsigned char* nombre2)
{
	int i;
	uint8_t igualdad=1;
	for(i=0; i<17 && igualdad ; i++)
	{
		if (nombre1[i]!=nombre2[i])
			igualdad=0;
		else if (nombre1[i]=='\0' && nombre2[i]=='\0')
				break;
	}
	return igualdad;
}

void copiarNombreArchivo(char* destino, int archivo) //Precondición: destino tiene 18 bytes o más
{
	int i;
	for (i=0; i<17 && tablaArchivos[archivo].fname[i]!='\0'; destino[i]=tablaArchivos[archivo].fname[i], i++);
	destino[i]='\0';
	return;
}

int buscarIndice(unsigned char* archivo, int dirPadre) //Para ignorar dirPadre, asignarle un negativo.
{
	int i;
	for (i=0;i<2048;i++)
	{
		if (sonNombresIguales(tablaArchivos[i].fname, archivo))
			if (dirPadre < 0 || tablaArchivos[i].parent_directory == dirPadre)
				return i;
	}
	return -1;
}

int obtenerPrimerBloque(int indice)
{
	if (indice<0 || indice>=2048)
		return -1;
	else
		return tablaAsignaciones[tablaArchivos[indice].first_block];
}

int obtenerBloqueSgte(int bloqueActual)
{
	if (bloqueActual<0)
		return -1;
	else
		return tablaAsignaciones[bloqueActual];
}

int liberarBloque(int bloque, uint8_t eliminarEnlace)
{
	if (bloque<0)
		return -1;

	if (eliminarEnlace)
	{
		int i;

		for(i=0;i<header->data_blocks && i>=0;i++)
		{
			if (tablaAsignaciones[i]==bloque)
			{
				tablaAsignaciones[i]=tablaAsignaciones[bloque];
				i=-2;
			}
		}
	}
	marcarBloque(bloque, 0);

	return 1;
}

int borrarArchivo(int indice)
{
	if (indice>=0 && indice<2048)
	{
		if (tablaArchivos[indice].state==DIRECTORY)
		{
			int i;
			for(i=0;i<2048;i++)
			{
				if (tablaArchivos[i].parent_directory == indice && tablaArchivos[i].state!=DELETED)
					return -2;
			}
		}
		else
			truncarArchivo(indice, 0);
		tablaArchivos[indice].state=DELETED;
	}
	else
		return -1;

	return 0;
}

int* obtenerBloques(int indice, int* size) //Devuelve en orden los índices de los bloques que conforman el archivo.
{
	int* array;
	int tamanio=0;
	if (indice<0)
	{
		*size=-1;
		return NULL;
	}
	else
	{
		if (tablaArchivos[indice].first_block==BLOQUE_NULO)
		{
			*size=0;
			return NULL;
		}
		else
		{
			array=malloc(sizeof(int));
			if (array==NULL) return NULL; //Verificacion de malloc
			array[0]=tablaArchivos[indice].first_block;
			int i = 0;
			tamanio++;
			while(tablaAsignaciones[array[i]]!=BLOQUE_NULO)
			{
				array=realloc(array, tamanio*sizeof(int)+sizeof(int));
				if (array==NULL) //Verificacion de realloc
				{
					*size=-1;
					return NULL;
				}
				tamanio++;
				array[i+1]=tablaAsignaciones[array[i]];
				i++;
			}
			(*size)=tamanio;
			return array;
		}
	}
}

int obtenerArchivo(char* path)
{
	int i = 0;
	int retorno;
	while(path[i]!=0 && path[i]=='/')
		i++;
	if (i==strlen(path))
		retorno = -1;
	else
	{
		i = 0;
		char** directorios=NULL;
		int dirActual=DIRECTORIO_NULO;
		directorios = string_split(path, "/");

		while(directorios[i]!=NULL)
		{
			dirActual = buscarIndice(directorios[i], dirActual);
			free(directorios[i]);
			if (dirActual<0)
				retorno = -1;
			i++;
		}

		if (!i || retorno==-1)
			retorno = -1;
		else
			retorno = dirActual;

		free(directorios);
	}
	return retorno;
}

int obtenerDirectorioPadre(char* path)
{
	int i = 0;
	int retorno = 0;
	while(path[i]!=0 && path[i]=='/')
		i++;
	if (i==strlen(path))
		retorno = -1;
	else
	{
		i = 0;
		char** directorios=NULL;
		int dirActual=DIRECTORIO_NULO;
		directorios = string_split(path, "/");
		if (directorios[0]==NULL)
			retorno = -1;
		else
		{
			while(directorios[i+1]!=NULL)
			{
				dirActual = buscarIndice(directorios[i], dirActual);
				free(directorios[i]);
				if (dirActual<0)
					retorno = -1;
				i++;
			}
			free(directorios[i]);
			retorno = retorno<0 ? -1 : dirActual;
		}
		free(directorios);
	}
	return retorno;
}

char* obtenerNombreArchivo(char* path) //Nombre del archivo según ruta, no según tabla de archivos. Hacer free de la variable devuelta
{
	int i = 0;
	char* retorno=NULL;
	char* nombre = "\0";
	while(path[i]!=0 && path[i]=='/')
		i++;
	if (i==strlen(path))
		retorno = nombre;
	else
	{
		i = 1;
		char** directorios=NULL;

		directorios = string_split(path, "/");
		if (directorios[0]==NULL)
			retorno = nombre;
		else
		{
			while(directorios[i]!=NULL)
				i++;

			nombre = malloc(strlen(directorios[i-1])+1);
			strcpy(nombre, directorios[i-1]);
			retorno = nombre;
			for(i=0;directorios[i]!=NULL;i++)
				free(directorios[i]);
		}
		free(directorios);
	}
	return retorno;
}

int encontrarBloqueLibre()
{
	int i;
	for(i = tamanioAdmin; bitarray_test_bit(&bitmap, i) && i<header->fs_blocks;i++);

	if (i<header->fs_blocks)
		return i-tamanioAdmin;
	else
		return -1;
}

int reservarNuevoBloque(int bloqueAnterior)
{
	int nuevoBloque=encontrarBloqueLibre();
	if (nuevoBloque<0)
		return -1;

	marcarBloque(nuevoBloque, 1);
	if (bloqueAnterior>=0)
		tablaAsignaciones[bloqueAnterior]=nuevoBloque;

	tablaAsignaciones[nuevoBloque]=BLOQUE_NULO;
	return nuevoBloque;
}

int guardarDesdeBloque(int bloqueActual, void* datos, int cantBytes, int offset)
{
	osada_block bloqueAux;
	memcpy(bloqueAux, inicioDatos[bloqueActual], OSADA_BLOCK_SIZE);
	int bytesCopiados= cantBytes>OSADA_BLOCK_SIZE-offset ? OSADA_BLOCK_SIZE-offset : cantBytes;
	memcpy(bloqueAux+offset, datos, bytesCopiados);
	int bloqueSgte;

	if (cantBytes!=bytesCopiados)
	{
		if ((bloqueSgte = obtenerBloqueSgte(bloqueActual))==BLOQUE_NULO)
			if ((bloqueSgte = reservarNuevoBloque(bloqueActual))<0)
				return 0;

		if(!guardarDesdeBloque(bloqueSgte, datos+bytesCopiados, cantBytes-bytesCopiados, 0))
			return 0;
	}
	guardarBloque(bloqueAux, bloqueActual);
	return 1;
}

unsigned char* concatenarBloques(int* indicesBloques, int arraySize) //Hacer free de la variable a la que se asigna
{
	unsigned char* contenido;
	int i;

	contenido = malloc(OSADA_BLOCK_SIZE*arraySize);
	memset(contenido, 0, arraySize*OSADA_BLOCK_SIZE);

	for(i=0; i<arraySize;i++)
		memcpy(contenido+(i*OSADA_BLOCK_SIZE), inicioDatos[indicesBloques[i]], OSADA_BLOCK_SIZE);

	return contenido;
}

int actualizar(int archivo, void* nuevosDatos, size_t cantidadBytes, uint64_t offset)
{
	int retorno = 1;
	if (tablaArchivos[archivo].state != REGULAR || tablaArchivos[archivo].file_size < offset)
		return 0;

	int nroBloques = 0;
	int bloqueInicial = offset/OSADA_BLOCK_SIZE;
	int* bloques = obtenerBloques(archivo, &nroBloques);

	if(bloqueInicial==nroBloques && nroBloques!=0)
	{
		bloqueInicial = reservarNuevoBloque(bloques[nroBloques-1]);
		if (bloqueInicial<0)
		{
			if(bloques!=NULL)
				free(bloques);
			return 0;
		}
	}
	else
	{
		if(bloqueInicial==nroBloques && nroBloques==0)
		{
			bloqueInicial = reservarNuevoBloque(-1);
			if (bloqueInicial<0)
				retorno = 0;
			tablaArchivos[archivo].first_block = bloqueInicial;
		}
		else if (bloqueInicial>nroBloques)
				retorno = 0;
			 else
				 bloqueInicial=bloques[bloqueInicial];
	}

	if (retorno)
	{
		if (guardarDesdeBloque(bloqueInicial, nuevosDatos, cantidadBytes, offset%OSADA_BLOCK_SIZE))
		{
			if (cantidadBytes + offset > tablaArchivos[archivo].file_size)
				tablaArchivos[archivo].file_size = cantidadBytes + offset;
			tablaArchivos[archivo].lastmod = time(NULL);
		}
		else
			retorno = 0;
	}

	free(bloques);
	return retorno;
}

int truncarArchivo(int archivo, uint64_t size)
{
	if (archivo<0 || size<0)
		return -1;
	else
	{
		int cantidad = 0;
		int* bloques = obtenerBloques(archivo, &cantidad);
		int bloquesNecesarios = size/OSADA_BLOCK_SIZE + ((size%OSADA_BLOCK_SIZE)>0);
		for(;cantidad>bloquesNecesarios; cantidad--)
		{
			if (cantidad>1)
				liberarBloque(bloques[cantidad-1], 1);
			else
			{
				tablaArchivos[archivo].first_block = BLOQUE_NULO;
				liberarBloque(bloques[cantidad-1], 0);
			}
		}

		tablaArchivos[archivo].file_size = size;
		tablaArchivos[archivo].lastmod = time(NULL);
		free(bloques);
	}
	return size;
}

int crearArchivo(int dirPadre, const char* nombre, osada_file_state tipo)
{
	if (strlen(nombre)>17)
		return -2;
	if (dirPadre<0)
		return -1;
	int i, nuevoIndice = -1;
	for(i=2047;i>=0;i--)
	{
		if (sonNombresIguales(tablaArchivos[i].fname, nombre)  && (tablaArchivos[i].parent_directory == dirPadre) && tablaArchivos[i].state != DELETED)
			return -3;
		else if (tablaArchivos[i].state==DELETED)
			nuevoIndice = i;
	}

	if (nuevoIndice<0)
		return -4;

	tablaArchivos[nuevoIndice].state = tipo;
	tablaArchivos[nuevoIndice].first_block = BLOQUE_NULO;
	tablaArchivos[nuevoIndice].parent_directory = dirPadre;
	nombrarArchivo(nuevoIndice, nombre);
	tablaArchivos[nuevoIndice].lastmod = time(NULL);
	tablaArchivos[nuevoIndice].file_size = 0;
	return 0;
}

int renombrarArchivo(int archivo, char* nuevoNombre)
{
	if(archivo<0)
		return -1;

	if (strlen(nuevoNombre)>17)
		return -2;

	nombrarArchivo(archivo, nuevoNombre);
	tablaArchivos[archivo].lastmod = time(NULL);
	return 0;
}


void gestionarSocket(void* socket)
{
	int cliente = (int) socket;
	int resultado = 0;
	void* buffer;
	uint8_t operacion;
	uint8_t tamBuffer;
	int archivo;
	int dirPadre;
	char* nombre;
	while(1)
	{
		buffer = malloc(2);
		resultado= recv(cliente, buffer, 2, 0);
		if (resultado<=0)
		{
			fflush(stdout);
			printf("Cliente %d desconectado al recibir codigo de operacion\n\n", cliente);
			return;
		}
		memcpy(&operacion, buffer, 1);
		memcpy(&tamBuffer, buffer+1, 1);
		free(buffer);
		buffer=malloc(tamBuffer);
		if ((resultado = recv(cliente, buffer, tamBuffer, 0))<=0)
		{
			fflush(stdout);
			printf("Cliente %d desconectado al entrar en getattr.\n\n", cliente);
			return;
		}
		printf("Operacion %d sobre %s\n", operacion, (char*)buffer);
		pthread_mutex_lock(&sem_estructuras);
		switch(operacion)
		{
		case COD_GETATTR:
				archivo = obtenerArchivo((char*)buffer);
				free(buffer);
				buffer = malloc(1);
				if (archivo<0)
				{
					memset(buffer, 0, 1);
					send(cliente, buffer, 1, 0);
				}
				else
				{
					memset(buffer, 1, 1);
					send(cliente, buffer, 1, 0);
					free(buffer);
					buffer=malloc(sizeof(osada_file));
					memcpy(buffer,tablaArchivos+archivo, sizeof(osada_file));
					send(cliente, buffer, sizeof(osada_file), 0);
				}
				free(buffer);
				break;

		case COD_READDIR:
				if (strcmp(buffer, "/") == 0)
					archivo= DIRECTORIO_NULO;
				else
					archivo = obtenerArchivo((char*)buffer);
				int i;
				free(buffer);
				buffer = malloc(1);
				if (archivo>=0)
				{
					uint8_t primero = 1;
					for (i=0;i<2048;i++)
					{
						if (tablaArchivos[i].state != DELETED && tablaArchivos[i].parent_directory == archivo)
						{
							if (primero)
							{
								primero = 0;
								memset(buffer, 1, 1);
								send(cliente, buffer, 1, 0);
								free(buffer);
							}
							buffer = malloc(OSADA_FILENAME_LENGTH+1);
							copiarNombreArchivo((char*)buffer, i);
							send(cliente, buffer, OSADA_FILENAME_LENGTH+1, 0);
							free(buffer);
						}
					}

					if (primero)
					{
						memset(buffer, 0, 1);
						send(cliente, buffer, 1, 0);
					}
					else
					{
						buffer = malloc(OSADA_FILENAME_LENGTH+1);
						memset(buffer, 0, OSADA_FILENAME_LENGTH+1);
						send(cliente, buffer, OSADA_FILENAME_LENGTH+1, 0);
					}

				}
				else
				{
					signed char menosUno = -1;
					buffer=malloc(1);
					memcpy(buffer, &menosUno, 1);
					send(cliente, buffer, 1, 0);
				}
				free(buffer);
				break;

		case COD_READ: //ATENCIÓN: off_t ocupa 4 bytes acá, y 8 en el cliente. Tener en cuenta al hacer sizeof(off_t)
				archivo = obtenerArchivo((char*)buffer);
				free(buffer);
				buffer = malloc(sizeof(size_t)+sizeof(uint64_t));
				if ((resultado = recv(cliente, buffer, sizeof(size_t)+sizeof(uint64_t), 0))<=0)
				{
					printf("Cliente %d desconectado.\n\n", cliente);
					return;
				}
				size_t cantLeida;
				memcpy(&cantLeida, buffer, sizeof(cantLeida));
				uint64_t offsetLectura;
				memcpy(&offsetLectura, buffer+sizeof(cantLeida), sizeof(offsetLectura));
				free(buffer);
				//printf("Archivo: %d -- Size: %d -- Offset: %d\n\n", archivo, cantLeida, offsetLectura);
				int cantidad = 0;
				int *bloques = obtenerBloques(archivo, &cantidad);
				unsigned char* contenidoArchivo;
				if (cantLeida+offsetLectura>tablaArchivos[archivo].file_size)
					cantLeida = tablaArchivos[archivo].file_size - offsetLectura;
				if (offsetLectura>tablaArchivos[archivo].file_size)
					cantLeida=0;
				send(cliente, &cantLeida, sizeof(cantLeida), 0);
				if (cantidad>0 && offsetLectura<tablaArchivos[archivo].file_size)
				{
					buffer = malloc(cantLeida);
					contenidoArchivo=concatenarBloques(bloques, cantidad);
					memcpy(buffer, contenidoArchivo+offsetLectura, cantLeida);
					free(contenidoArchivo);
					free(bloques);
					send(cliente, buffer, cantLeida, 0);
					free(buffer);
				}
				break;

		case COD_TRUNCATE:
				archivo = obtenerArchivo((char*)buffer);
				free(buffer);
				buffer = malloc(sizeof(uint64_t));
				if ((resultado = recv(cliente, buffer, sizeof(uint64_t), 0))<=0)
				{
					printf("Cliente %d desconectado.\n\n", cliente);
					return;
				}
				uint64_t size;
				memcpy(&size, buffer, sizeof(size));
				free(buffer);
				truncarArchivo(archivo, size);
				break;

		case COD_WRITE:
				archivo = obtenerArchivo((char*)buffer);
				free(buffer);
				buffer = malloc(sizeof(size_t)+sizeof(uint64_t));
				if ((resultado = recv(cliente, buffer, sizeof(size_t)+sizeof(uint64_t), 0))<=0)
				{
					printf("Cliente %d desconectado.\n\n", cliente);
					return;
				}

				size_t tamEscritura;
				memcpy(&tamEscritura, buffer, sizeof(tamEscritura));
				uint64_t offsetEscritura;
				memcpy(&offsetEscritura, buffer+sizeof(tamEscritura), sizeof(offsetEscritura));
				free(buffer);
				buffer = malloc(tamEscritura);
				if ((resultado = recv(cliente, buffer, tamEscritura, 0))<=0)
				{
					printf("Cliente %d desconectado.\n\n", cliente);
					return;
				}
				resultado = actualizar(archivo, buffer, tamEscritura, offsetEscritura);
				free(buffer);
				buffer = malloc(1);
				if (resultado)
					memset(buffer, 1, 1);
				else
					memset(buffer, 0, 1);
				send(cliente, buffer, 1, 0);
				free(buffer);
				break;

		case COD_CREATE:; //No me deja declarar como primera instrucción -.-
				dirPadre = obtenerDirectorioPadre((char*)buffer);
				nombre = obtenerNombreArchivo((char*)buffer);
				free(buffer);
				buffer = malloc(1);
				if (strlen(nombre))
				{
					switch (crearArchivo(dirPadre, nombre, REGULAR))
					{
					case 0 : memset(buffer, 0, 1); break;
					case -1 : memset(buffer, 1, 1); break;
					case -2 : memset(buffer, 2, 1); break;
					case -3 : memset(buffer, 3, 1); break;
					case -4 : memset(buffer, 4, 1); break;
					}
				}
				else
					memset(buffer, 1, 1);
				send(cliente, buffer, 1, 0);
				free(nombre);
				free(buffer);
				break;

		case COD_MKDIR:;
				dirPadre = obtenerDirectorioPadre((char*)buffer);
				nombre = obtenerNombreArchivo((char*)buffer);
				free(buffer);
				buffer = malloc(1);
				if (strlen(nombre))
				{
					switch (crearArchivo(dirPadre, nombre, DIRECTORY))
					{
					case 0 : memset(buffer, 0, 1); break;
					case -1 : memset(buffer, 1, 1); break;
					case -2 : memset(buffer, 2, 1); break;
					case -3 : memset(buffer, 3, 1); break;
					case -4 : memset(buffer, 4, 1); break;
					}
				}
				else
					memset(buffer, 1, 1);
				send(cliente, buffer, 1, 0);
				free(nombre);
				free(buffer);
				break;

		case COD_RMDIR:
				archivo = obtenerArchivo(buffer);
				free(buffer);
				buffer = malloc(1);
				switch (borrarArchivo(archivo))
				{
				case 0: memset(buffer, 0, 1); break;
				case -1: memset(buffer, 1, 1); break;
				case -2: memset(buffer, 2, 1); break;
				}
				send(cliente, buffer, 1, 0);
				free(buffer);
				break;

		case COD_UNLINK:
				archivo = obtenerArchivo(buffer);
				free(buffer);
				buffer = malloc(1);
				switch (borrarArchivo(archivo))
				{
				case 0: memset(buffer, 1, 1); break;
				case -1: memset(buffer, 0, 1); break;
				}
				send(cliente, buffer, 1, 0);
				free(buffer);
				break;

		case COD_RENAME:
				archivo =obtenerArchivo((char*)buffer);
				free(buffer);
				buffer = malloc(1);
				if ((recv(cliente,buffer,1,0))<=0)
				{
					printf("Cliente %d desconectado.\n\n", cliente);
					return;
				}
				uint8_t tamPath=0;
				memcpy(&tamPath, buffer, 1);
				free(buffer);
				buffer=malloc(tamPath);
				if ((recv(cliente,buffer,tamPath,0))<=0)
				{
					printf("Cliente %d desconectado.\n\n", cliente);
					return;
				}

				int archivoNuevo = obtenerArchivo((char*)buffer);
				nombre = obtenerNombreArchivo((char*)buffer);
				dirPadre = obtenerDirectorioPadre((char*)buffer);
				free(buffer);
				buffer=malloc(1);
				if (archivoNuevo<0)
				{
					if (strlen(nombre)>0)
					{	switch(renombrarArchivo(archivo,nombre))
						{
						case 0 : memset(buffer, 0, 1); tablaArchivos[archivo].parent_directory = dirPadre; break;
						case -1 : memset(buffer, 1, 1); break;
						case -2 : memset(buffer, 2, 1); break;
						}
					}
				}
				else
					memset(buffer, 3, 1);

				send(cliente, buffer, 1, 0);
				free(nombre);
				free(buffer);
				break;
		default: free(buffer);
		}
		pthread_mutex_unlock(&sem_estructuras);
	}
	return;
}

int main(int argc, char** argv)
{
//	int fd_fileSystem = open(argv[1], 2); //2 significa O_RDWR, leer y escribir
	int fd_fileSystem = open("juego.bin", 2); //Para debuggear
	if (fd_fileSystem==-1)
	{
		printf("Archivo de file system no encontrado.\n");
		exit(0);
	}

//	int puerto = strtol(argv[2], NULL, 10);
	int puerto =19000;
	if(puerto<0)
	{
		printf("Puerto no válido.\n");
		exit(0);
	}
	struct stat fsStat;
	fstat(fd_fileSystem, &fsStat);

	header= mmap(0, fsStat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_fileSystem, 0);


	bitmap.size = header->bitmap_blocks * OSADA_BLOCK_SIZE;
	bitmap.bitarray=(char*)header;
	bitmap.bitarray+=OSADA_BLOCK_SIZE;
	bitmap.mode=MSB_FIRST;

	uint8_t* aux = (uint8_t*)header;
	aux+= OSADA_BLOCK_SIZE + bitmap.size;
	tablaArchivos = (osada_file*)aux;

	aux+= 2048*sizeof(osada_file);
	tablaAsignaciones = (osada_block_pointer*)aux;
	tamanioAsig = header->fs_blocks - header->allocations_table_offset - header->data_blocks; //En bloques

	aux += tamanioAsig*OSADA_BLOCK_SIZE;
	inicioDatos= (osada_block*)aux;

	tamanioAdmin = header->fs_blocks - header->data_blocks;

//************************************************

	fd_set master;   // conjunto maestro de descriptores de fichero
	fd_set read_fds; // conjunto temporal de descriptores de fichero para select()

	int fdmax;        // número máximo de descriptores de fichero
	int listener;     // descriptor de socket a la escucha
	int i;
	pthread_t hilo;
	FD_ZERO(&master);    // borra los conjuntos maestro y temporal
	FD_ZERO(&read_fds);

	listener = socket_startListener(puerto);

	// añadir listener al conjunto maestro
	FD_SET(listener, &master);

	// seguir la pista del descriptor de fichero mayor
	fdmax = listener; // por ahora es éste

	// bucle principal
		while(1)
		{
			read_fds = master;

			//Buscamos los sockets que quieren realizar algo con Select
			socket_select(fdmax, &read_fds);

			// explorar conexiones existentes en busca de datos que leer
			for(i = 0; i <= fdmax; i++)
			{

				if (FD_ISSET(i, &read_fds))
				{
					if (i == listener)
					{
						//ACEPTAMOS UN NUEVO CLIENTE
						//socket_addNewConnection lo que ace
						int cliente = socket_addNewConection(listener,&master,&fdmax);
						int retval = pthread_create(&hilo, NULL, (void*)gestionarSocket, (void*) cliente);
						if(!retval)
							printf("Nueva conexion detectada.Socket: %d\n", cliente);
						else
							printf("No se ha podido establecer la conexion con el cliente.Socket: %d\n", cliente);
						//TODO revisar desconexion de cliente
					}
				}
			}
		}

		return 0;
}
