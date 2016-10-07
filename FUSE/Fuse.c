/*
 * FUSE.c
 *
 *  Created on: 19/9/2016
 *      Author: utnso
 */

/* Este es un programa de prueba para intentar entender cómo funciona FUSE.
 * Los comentarios muestran lo que se sabe hasta el momento.
 *
 * Antes que nada, es importante destacar que Linux considera que cualquier cosa es un archivo (incluso los
 * directorios), asi que cuando se mencione la palabra "archivo" no se refiere al archivo de datos
 * tradicional (archivo simple).
 *
 * Al ejecutar desde consola: ./FUSE "carpeta punto de montaje" -f (recomendable) -d (opcional)
 * -f: ejecuta en primer plano, por lo que la consola quda disponible para enviar señales.
 * -d: muestra las operaciones realizadas por FUSE. Puede ser útil para probar cosas.
 * La carpeta punto de montaje debe estar creada, y puede pasarse como ruta relativa.
 *
 * Revisar repo de github para saber qué linkear: https://github.com/sisoputnfrba/so-fuse_example*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fuse.h>
#include <stddef.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "headers/osada.h"
#include <commons/bitarray.h>
#include <commons/string.h>

#define BLOQUE_NULO 0xFFFFFFFF
#define DIRECTORIO_NULO 0xFFFF


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
void guardarBloque(osada_block, int);
osada_block_pointer* obtenerPtroBloque(int);
int buscarIndice(unsigned char*, int);
int obtenerPrimerBloque(int);
int obtenerBloqueSgte(int);
int encontrarBloqueLibre();
int liberarBloque(int, uint8_t);
void borrarArchivo(int);
int* obtenerBloques(int, int*);
int obtenerArchivo(char*);
unsigned char* concatenarBloques(int*, int);
int reservarNuevoBloque(int);
int guardarDesdeBloque(int, void*, int, int);
int actualizar(int, void*, int, int);


void marcarBloque(osada_block_pointer bloque, uint8_t valor)
{
	if (valor)
		bitarray_set_bit(&bitmap, bloque+tamanioAdmin);
	else
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

unsigned char* concatenarBloques(int* indicesBloques, int arraySize)
{
	unsigned char* contenido;
	int i;

	contenido = malloc(OSADA_BLOCK_SIZE*arraySize);
	memset(contenido, 0, arraySize*OSADA_BLOCK_SIZE);

	for(i=0; i<arraySize;i++)
		memcpy(contenido+(i*OSADA_BLOCK_SIZE), inicioDatos[indicesBloques[i]], OSADA_BLOCK_SIZE);

	return contenido;
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

int liberarBloque(int bloque, uint8_t eliminarEnlace)
{
	if (bloque<0)
		return -1;

	if (eliminarEnlace)
	{
		int i;

		for(i=0;i<header->data_blocks, i>=0;i++)
		{
			if (tablaAsignaciones[i]==bloque)
			{
				printf("Bloque vale: %d. El bloque que lo contiene es %d.\n\n", bloque, i);
				tablaAsignaciones[i]=tablaAsignaciones[bloque];
				i=-2;
			}
		}
	}
	marcarBloque(bloque, 0);

	return 1;
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

int actualizar(int archivo, void* nuevosDatos, int cantidadBytes, int offset)
{
	if (tablaArchivos[archivo].state != REGULAR || tablaArchivos[archivo].file_size < offset)
		return 0;

	int nroBloques = tablaArchivos[archivo].file_size / OSADA_BLOCK_SIZE + ((tablaArchivos[archivo].file_size % OSADA_BLOCK_SIZE)>0);
	int bloqueInicial = 0;

	if(nroBloques==0)
	{
		bloqueInicial = reservarNuevoBloque(-1);
		if (bloqueInicial<0)
			return 0;
		tablaArchivos[archivo].first_block = bloqueInicial;
	}

	int* bloques = obtenerBloques(archivo, &bloqueInicial); //Uso bloqueInicial porque no me sirve la cantidad
	bloqueInicial = nroBloques==0 ? 0 : offset/(nroBloques*OSADA_BLOCK_SIZE);


	if (guardarDesdeBloque(bloques[bloqueInicial], nuevosDatos, cantidadBytes, offset))
	{
		if (cantidadBytes + offset > tablaArchivos[archivo].file_size)
			tablaArchivos[archivo].file_size = cantidadBytes + offset;
	}
	else
		return 0;

	return 1;
}


static int osada_getattr(const char *path, struct stat *stbuf) {

	int res = 0;
	memset(stbuf, 0, sizeof(struct stat));

	if (strcmp(path, "/") == 0)
	{
		stbuf->st_mode = S_IFDIR | 0777;
		stbuf->st_nlink = 2;
	}
	else
	{
		if ((res = obtenerArchivo(path))<0)
			return -ENOENT;
		else
		{
			if (tablaArchivos[res].state == DIRECTORY)
			{
				stbuf->st_mode = S_IFDIR | 0777;
				stbuf->st_nlink = 2;
			}
			else if (tablaArchivos[res].state == REGULAR)
			{
				stbuf->st_mode = S_IFREG | 0777;
				stbuf->st_nlink = 1;
				stbuf->st_size = (__off_t)tablaArchivos[res].file_size;
			}
			else
				return -ENOENT;
		}
	}

	return 0;
}


static int osada_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {

	int res=0;

	if (strcmp(path, "/") == 0)
	{
		int i;
		for (i=0;i<2048;i++)
		{
			if (tablaArchivos[i].state != DELETED && tablaArchivos[i].parent_directory == DIRECTORIO_NULO)
				filler(buf, tablaArchivos[i].fname, NULL, 0);
		}
	}
	else if ((res=obtenerArchivo(path))<0)
		return -ENOENT;
	else if (tablaArchivos[res].state == DELETED)
		return -ENOENT;
	else
	{
		filler(buf, ".", NULL, 0);
		filler(buf, "..", NULL, 0);
		int i;
		for (i=0;i<2048;i++)
		{
			if (tablaArchivos[i].state != DELETED && tablaArchivos[i].parent_directory == res)

				filler(buf, tablaArchivos[i].fname, NULL, 0);
		}
	}
	return 0;
}

static int osada_read(const char *path, char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi) {

	int archivo = obtenerArchivo(path);
	int cantidad = 0;
	int *bloques = obtenerBloques(archivo, &cantidad);
	unsigned char* contenidoArchivo;

	contenidoArchivo=concatenarBloques(bloques, cantidad);
	memcpy(buf, contenidoArchivo+offset, size);
	free(contenidoArchivo);
	if (cantidad>0)
		free(bloques);
	return size;
}

int osada_truncate(const char * path, off_t size) //Truncate debe estar para que write funcione
{
	int archivo = obtenerArchivo(path);
	if (archivo<0)
		return -ENOENT;
	else
	{

		int cantidad=0;
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
	}

	return 0;
}

static int osada_write(const char *path, char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi) {

	int archivo = obtenerArchivo(path);

	if (!actualizar(archivo, buf, size, offset))
		return 0;

	return size;
}

static struct fuse_operations osada_oper = {
		.getattr = osada_getattr,
		.readdir = osada_readdir,
		.read = osada_read,
		.truncate = osada_truncate,
		.write = osada_write,
};


int main(int argc, char *argv[])
{
	int fd_fileSystem = open("../basic.bin", O_RDWR);
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

	//El núcleo de FUSE. En teoría hay que invocarlo y olvidarse.
	return fuse_main(argc, argv, &osada_oper, NULL);

	return 0;
}
