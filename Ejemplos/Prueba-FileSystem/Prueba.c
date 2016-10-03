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
void marcarBloque(osada_block_pointer, uint8_t);
void guardarDato(osada_block, int);
osada_block_pointer* obtenerPtroBloque(int);
int buscarIndice(unsigned char*, int);
osada_block_pointer* obtenerPrimerBloque(int);
void borrarArchivo(int);
int* obtenerBloques(int, int*);
void barrer();
int obtenerArchivo(char*);


void marcarBloque(osada_block_pointer bloque, uint8_t valor)
{
	if (valor) //Se realizan operaciones a nivel bit (bitwise) para distinguir bits dentro de los bytes del bitmap
		//bitmap[(bloque)/8]= ((unsigned char)pow(2,(8-(bloque)%8)-1)) | (bitmap[(bloque)/8]);
		bitarray_set_bit(&bitmap, bloque);
	else
		//bitmap[(bloque)/8]= ~((unsigned char)pow(2,(8-(bloque)%8)-1)) & (bitmap[(bloque)/8]);
		bitarray_clean_bit(&bitmap, bloque);
}

void guardarDato(osada_block dato, int bloque) //Podría agregarse la búsqueda de un bloque libre
{
	memset(inicioDatos[bloque], 0, OSADA_BLOCK_SIZE);
	memcpy(inicioDatos[bloque], dato, OSADA_BLOCK_SIZE);
	marcarBloque(bloque+tamanioAdmin, 1);
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

osada_block_pointer* obtenerPrimerBloque(int indice)
{
	if (indice<0)
		return NULL;
	else
		return tablaAsignaciones[tablaArchivos[indice].first_block];
}

void borrarArchivo(int indice)
{
	tablaArchivos[indice].state=DELETED;
	int tamanio=0;
	int* bloques=obtenerBloques(indice, &tamanio);
	tamanio--;
	for(;tamanio>0;tamanio--)
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
	while(path[i]!=0 && path[i]=='/') //Verifico esto porque string_split rompe si son todas '/'
		i++;
	if (i==strlen(path))
		return-1;
	char** directorios=NULL;
	int dirAnterior=DIRECTORIO_NULO;
	directorios = string_split(path, "/"); //Crea una subcadena cada vez que encuentra una '/'

	while(directorios[i]!=NULL)
	{
		dirAnterior = buscarIndice(directorios[i], dirAnterior);
		if (dirAnterior<0)
			return -1;
		i++;
	}

	if (i==0)
		return -1;
	else
		return dirAnterior;
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
			printf("Bloques tabla Asignacion: ");
			int size=0;
			int* bloques;
			bloques= obtenerBloques(i, &size);
			int j;
			if (bloques!=NULL)
			{
				//Para realizar el challenge. SACAR EL FWRITE DEL FOR SI NO SE USA!!!
				FILE* archivoLeido;
				char* challenge= malloc(28);
				strcpy(challenge, "Challenge/\0");
				archivoLeido= fopen(strcat(challenge, tablaArchivos[i].fname), "wb");
				for(j=0;j<size;j++)
				{
					//printf("%d (primeros 10 bytes: %.*s)-> ", bloques[j], 10, obtenerPtroBloque(bloques[j]));
					if ((size==j+1) && (tablaArchivos[i].file_size % 64 > 0))
						fwrite(inicioDatos[bloques[j]], tablaArchivos[i].file_size % 64, 1, archivoLeido);
					else
						fwrite(inicioDatos[bloques[j]], 64, 1, archivoLeido);
				}
				free(challenge);
				free(bloques);
				fclose(archivoLeido);
			}
			printf("\n");
			if (tablaArchivos[i].parent_directory!=DIRECTORIO_NULO)
				printf("Esta adentro de: %.*s\n", 17, tablaArchivos[tablaArchivos[i].parent_directory].fname);
			printf("-------------\n");

		}
	}
}

int main()
{
	int fd_fileSystem = open("../challenge.bin", O_RDWR);
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
	int j=0;
	for (j=0;j<bitmap.size;j++)
	{
		printf("%d", bitmap.bitarray[j]);
		if (!((j+1)%4)) printf (" ");
		if (!((j+1)%8)) printf ("\n");

	}

	barrer();
	munmap(header, fsStat.st_size);
	close(fd_fileSystem);

	return 0;
}
