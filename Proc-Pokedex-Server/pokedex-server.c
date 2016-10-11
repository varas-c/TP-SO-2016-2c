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
#include <commons/config.h>
#include <commons/log.h>
#include <commons/bitarray.h>
#include <commons/string.h>
#include "headers/struct.h"
#include "headers/osada.h"
#include "headers/socket.h"


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
int liberarBloque(int, uint8_t);
void borrarArchivo(int);
int* obtenerBloques(int, int*);
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

/*
leer_archivo_osada(FILE* f,osada_header* header, osada_file* file)
{
	//HEADER
	header = malloc(sizeof(osada_header*));
	fread(header->magic_number, 7, 1, f);
	fread(header->version, 1, 1, f);
	fread(header->fs_blocks, 4, 1, f);
	fread(header->bitmap_blocks, 4, 1, f);
	fread(header->allocations_table_offset, 4, 1, f);
	fread(header->data_blocks, 4, 1, f);
	fread(header->padding, 40, 1, f);

	printf("\n%s\n",header->magic_number);

	//FILE
	file=malloc(sizeof(osada_file));
	osada_file_state state;
	fread(&state, 1, 1, f);
	file->state=state;
	fread(file->fname, 17, 1, f);
	fread(file->parent_directory, 2, 1, f);
	fread(file->file_size, 4, 1, f);
	fread(file->lastmod, 4, 1, f);
	fread(file->first_block, 4, 1, f);
}
*/

void gestionarSocket(void* socket)
{
	int cliente = (int) socket;
	int resultado = 0;
	void* buffer;
	uint8_t operacion;
	uint8_t tamBuffer;
	int archivo;
	while(1)
	{
		buffer = malloc(2);
		resultado= recv(cliente, buffer, 2, 0);
		if (resultado<=0)
			return;
		memcpy(&operacion, buffer, 1);
		memcpy(&tamBuffer, buffer+1, 1);
		free(buffer);
		buffer=malloc(tamBuffer);
		switch(operacion)
		{
		case 0: if ((resultado = recv(cliente, buffer, tamBuffer, 0))<=0)
					return;
				archivo = obtenerArchivo((char*)buffer);
				free(buffer);
				if (archivo<0)
				{
					buffer = malloc(1);
					signed char menosUno = -1;
					memcpy(buffer, &menosUno, 1);
					send(cliente, buffer, 1, 0);
				}
				else
				{
					buffer=malloc(sizeof(osada_file));
					memcpy(buffer,tablaArchivos+archivo, sizeof(osada_file));
					send(cliente, buffer, sizeof(osada_file), 0);
				}
				free(buffer);
				break;

		case 1: if ((resultado = recv(cliente, buffer, tamBuffer, 0))<=0)
				{
					printf("Cliente desconectado. Hilo cerrado\n\n");
					return;
				}
				if (strcmp(buffer, "/") == 0)
					archivo= DIRECTORIO_NULO;
				else
					archivo = obtenerArchivo((char*)buffer);
				int i;
				free(buffer);
				if (archivo>=0)
				{
					uint8_t primero = 1;
					for (i=0;i<2048;i++)
					{
						if (primero)
						{
							buffer = malloc(1);
							primero = 0;
							memset(buffer, 1, 1);
							send(cliente, buffer, 1, 0);
							free(buffer);
						}
						if (tablaArchivos[i].state != DELETED && tablaArchivos[i].parent_directory == archivo)
							send(cliente, (void*)tablaArchivos[i].fname, 17, 0);
					}
					buffer = malloc(17);
					memset(buffer, 0, 17);
					send(cliente, buffer, 17, 0);
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
		}
	}
	return;
}

int main(int argc, char** argv)
{
	int fd_fileSystem = open("../challenge.bin", 2); //2 significa O_RDWR, leer y escribir
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

//************************************************

	fd_set master;   // conjunto maestro de descriptores de fichero
	fd_set read_fds; // conjunto temporal de descriptores de fichero para select()

	int fdmax;        // número máximo de descriptores de fichero
	int listener;     // descriptor de socket a la escucha
	int i;
	pthread_t hilo;
	FD_ZERO(&master);    // borra los conjuntos maestro y temporal
	FD_ZERO(&read_fds);

	int puerto = 9995;
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
						int retval = pthread_create(&hilo, NULL, gestionarSocket, (void*) cliente);
						printf("Nueva conexion detectada.Socket: %d\n", cliente);
						//TODO revisar desconexion de cliente
					}
				}
			}
		}

		return 0;
}
