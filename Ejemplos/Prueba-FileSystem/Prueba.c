/*
 * Prueba.c
 *
 *  Created on: 9/9/2016
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "headers/osada.h"
#include <sys/stat.h>
#include <fuse.h>
#include <sys/mman.h>
#include <commons/bitarray.h>
#include <commons/string.h>

#define BLOQUE_NULO 0xFFFFFFFF
#define DIRECTORIO_NULO 0xFFFF


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


//**** PROTOTIPOS ****
int encontrarBloqueLibre();
void marcarBloque(osada_block_pointer, uint8_t);
void guardarBloque(osada_block, int);
osada_block_pointer* obtenerPtroBloque(int);
int buscarIndice(unsigned char*, int);
int obtenerPrimerBloque(int);
int obtenerBloqueSgte(int);
int reservarNuevoBloque(int);
void borrarArchivo(int);
int* obtenerBloques(int, int*);
void barrer();
int obtenerArchivo(char*);
unsigned char* concatenarBloques(int*, int);
int guardarDesdeBloque(int, void*, int, int);
int actualizar(int, void*, int, int);



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

int buscarIndice(unsigned char* archivo, int dirPadre) //Para ignorar dirPadre, asignarle un negativo.
{
	int i;
	for (i=0;i<2048;i++)
	{
		if (!strcmp(tablaArchivos[i].fname, archivo)) //Recordar que strcmp devuelve 0 si son iguales
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

void borrarArchivo(int indice)
{
	tablaArchivos[indice].state=DELETED;
	int tamanio=0;
	int* bloques=obtenerBloques(indice, &tamanio);
	for(--tamanio;tamanio>0;tamanio--)
		marcarBloque(bloques[tamanio], 0);
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
	while(path[i]!=0 && path[i]=='/')
		i++;
	if (i==strlen(path))
		return-1;
	else
		i = 0;
	char** directorios=NULL;
	int dirActual=DIRECTORIO_NULO;
	directorios = string_split(path, "/");

	while(directorios[i]!=NULL)
	{
		dirActual = buscarIndice(directorios[i], dirActual);
		if (dirActual<0)
			return -1;
		i++;
	}

	if (!i)
		return -1;
	else
		return dirActual;
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
	else
	{
		marcarBloque(nuevoBloque, 1);
		tablaAsignaciones[bloqueAnterior]=nuevoBloque;
	}
	return nuevoBloque;
}

int guardarDesdeBloque(int bloqueActual, void* datos, int cantBytes, int offset)
{
	osada_block bloqueAux;
	memcpy(bloqueAux, inicioDatos[bloqueActual], OSADA_BLOCK_SIZE);
	int bytesCopiados= cantBytes>OSADA_BLOCK_SIZE ? OSADA_BLOCK_SIZE-offset : cantBytes;
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

int actualizar(int archivo, void* nuevosDatos, int cantidadBytes, int offset)
{
	if (tablaArchivos[archivo].state != REGULAR || tablaArchivos[archivo].file_size < offset)
		return 0;

	int nroBloques = tablaArchivos[archivo].file_size / OSADA_BLOCK_SIZE + ((tablaArchivos[archivo].file_size % OSADA_BLOCK_SIZE)>0);

	if (guardarDesdeBloque(offset/(nroBloques*OSADA_BLOCK_SIZE), nuevosDatos, cantidadBytes, offset))
	{
		if (cantidadBytes + offset > tablaArchivos[archivo].file_size)
			tablaArchivos[archivo].file_size = cantidadBytes + offset;
	}
	else
		return 0;

	return 1;
}

void barrer()
{
	int i;
	printf("\n*** CONTENIDO DEL FILESYSTEM ***\n\n");
	printf("Version: %d\nTamanio: %d\nInicioTAS: %d\n\n", header->version, header->fs_blocks, header->allocations_table_offset);
	for(i=0;i<2048;i++)
	{
		if (tablaArchivos[i].state!=REGULAR && tablaArchivos[i].state!=DIRECTORY)
			//tablaArchivos[i].state=DELETED
			1;
		else
		{
			printf("Nombre: %s\nTipo: %d\nTamanio:%d\n", tablaArchivos[i].fname, tablaArchivos[i].state, tablaArchivos[i].file_size);
			int size=0;
			int* bloques;
			bloques= obtenerBloques(i, &size);
			if (bloques!=NULL)
			{
				unsigned char* contenido = concatenarBloques(bloques, size);
				printf("\nContenido: \n%.*s\n", size*OSADA_BLOCK_SIZE, contenido);
/*				//Para crear los archivos del challenge
				FILE* archivoLeido;
				char* challenge= malloc(28);
				strcpy(challenge, "Challenge/\0");
				archivoLeido= fopen(strcat(challenge, tablaArchivos[i].fname), "wb");
				int j;
				for(j=0;j<size;j++)
				{
					if ((size==j+1) && (tablaArchivos[i].file_size % 64 > 0))
						fwrite(inicioDatos[bloques[j]], tablaArchivos[i].file_size % 64, 1, archivoLeido);
					else
						fwrite(inicioDatos[bloques[j]], 64, 1, archivoLeido);
				}
				free(challenge);
				fclose(archivoLeido);*/
				free(contenido);
				free(bloques);
			}
			printf("\n");
			if (tablaArchivos[i].parent_directory!=DIRECTORIO_NULO)
				printf("Esta adentro de: %s\n", tablaArchivos[tablaArchivos[i].parent_directory].fname);
			printf("-------------\n");

		}
	}
}

int main()
{
	int fd_fileSystem = open("basic.bin", O_RDWR);
	if (fd_fileSystem==-1)
	{
		printf("Archivo de file system no encontrado.\n");
		exit(0);
	}
	struct stat fsStat;
	fstat(fd_fileSystem, &fsStat);

	header= mmap(0, fsStat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_fileSystem, 0);


	bitmap.size = header->bitmap_blocks * OSADA_BLOCK_SIZE;
	bitmap.bitarray=header;
	bitmap.bitarray+=OSADA_BLOCK_SIZE;
	bitmap.mode=MSB_FIRST;

	uint8_t* aux = header;
	aux+= OSADA_BLOCK_SIZE + bitmap.size;
	tablaArchivos = (osada_file*)aux;

	aux+= 2048*sizeof(osada_file);
	tablaAsignaciones = aux;
	tamanioAsig = header->fs_blocks - header->allocations_table_offset - header->data_blocks; //En bloques

	aux += tamanioAsig*OSADA_BLOCK_SIZE;
	inicioDatos= aux;

	tamanioAdmin = header->fs_blocks - header->data_blocks;

	//TODO Borrar cuando ya no sea necesario. (Muestra el bitmap)
/*	int j=0;
	for (j=0;j<bitmap.size;j++)
	{
		printf("%d", bitmap.bitarray[j]);
		if (!((j+1)%4)) printf (" ");
		if (!((j+1)%8)) printf ("\n");

	}
*/
	barrer();

	actualizar(1, (void*)"She had something to confess to, but you don't have the time, so look the other way", 93, 3);
	printf("Dato: %.*s", 93, inicioDatos[0]);
	munmap(header, fsStat.st_size);
	close(fd_fileSystem);

	return 0;
}
