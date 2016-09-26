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
#include <math.h>
#include <commons/bitarray.h>


/* TAMAÑOS ESTRUCTURAS [BLOQUES]: (F= tamaño filesystem)
 *
 * HEADER = 1 (H)
 * BITMAP = F / 512 (/8/64) (B)
 * TABLA ARCHIVOS = 1024 (TAR)
 * TABLA ASIGACIONES = (F - H - B - TAR) / 16 (TAS)
 * DATOS = (F - H - B - TAR - TAS)
 *
 */

osada_header* header;
t_bitarray bitmap;
osada_file* tablaArchivos;
osada_block_pointer* tablaAsignaciones;
int tamanioAsig;
osada_block* inicioDatos;
int tamanioAdmin;

void listar()
{
	int i;
	printf("\n*** CONTENIDO DEL FILESYSTEM ***\n\n");
	printf("Version: %d\nTamanio: %d\nInicioTAS: %d\n\n", header->version, header->fs_blocks, header->allocations_table_offset);
	for(i=0;i<2048;i++)
	{
		if (tablaArchivos[i].state!=REGULAR && tablaArchivos[i].state!=DIRECTORY)
			tablaArchivos[i].state=DELETED;
		else
		{
			printf("Nombre: %s\nTipo: %d\nTamanio:%d\n", tablaArchivos[i].fname, tablaArchivos[i].state, tablaArchivos[i].file_size);
			printf("Bloques tabla Asignacion: ");
			int j=tablaArchivos[i].first_block;
			while(j!=-1)
			{
				printf("%d (dato: %s)-> ", j, inicioDatos[j-tamanioAsig]);
				j = tablaAsignaciones[j];
			}
			printf("\n");

			if (tablaArchivos[i].parent_directory!=0xFFFF)
				printf("Esta adentro de: %s\n", tablaArchivos[tablaArchivos[i].parent_directory].fname);
			printf("-------------\n");

		}
	}
}

void marcarBloque(osada_block_pointer bloque, uint8_t valor)
{
	if (valor) //Se realizan operaciones a nivel bit (bitwise) para distinguir bits dentro de los bytes del bitmap
		//bitmap[(bloque)/8]= ((unsigned char)pow(2,(8-(bloque)%8)-1)) | (bitmap[(bloque)/8]);
		bitarray_set_bit(&bitmap, bloque);
	else
		//bitmap[(bloque)/8]= ~((unsigned char)pow(2,(8-(bloque)%8)-1)) & (bitmap[(bloque)/8]);
		bitarray_clean_bit(&bitmap, bloque);
}

void guardarDato(osada_block dato, osada_block_pointer bloque) //Podría agregarse la búsqueda de un bloque libre
{
	memcpy(inicioDatos[bloque-tamanioAdmin], dato, OSADA_BLOCK_SIZE);
}

int main()
{
/*	FILE* fileSystem = NULL;
	fileSystem = fopen("/home/utnso/workspace/Prueba-FileSystem/prueba.bin", "rw");

	osada_header header;
	fread(&header, sizeof(header), 1, fileSystem);

	int tamanioBitmap = header.bitmap_blocks * OSADA_BLOCK_SIZE;
	uint8_t bitmap[tamanioBitmap];
	fread(&bitmap, sizeof(bitmap), 1, fileSystem);

	osada_file tablaArchivos[2048];
	fread(&tablaArchivos, sizeof(tablaArchivos), 1, fileSystem);

	osada_block_pointer tablaAsignaciones[header.data_blocks];
	fread(&tablaAsignaciones, sizeof(tablaAsignaciones), 1, fileSystem);

	fclose(fileSystem);
*/

	int fd_fileSystem = open("/home/utnso/workspace/Prueba-FileSystem/prueba.bin", O_RDWR);
	struct stat fsStat;
	fstat(fd_fileSystem, &fsStat);

	header= mmap(0, fsStat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_fileSystem, 0);


	bitmap.size = header->bitmap_blocks * OSADA_BLOCK_SIZE;
	bitmap.bitarray=header;
	bitmap.bitarray+=OSADA_BLOCK_SIZE;
	bitmap.mode=MSB_FIRST;

	uint8_t* aux = header + OSADA_BLOCK_SIZE + bitmap.size;
	tablaArchivos = (osada_file*)aux;


	aux += header->allocations_table_offset*OSADA_BLOCK_SIZE;
	tablaAsignaciones = aux;
	tamanioAsig = header->fs_blocks - header->allocations_table_offset - header->data_blocks; //En bloques

	aux += tamanioAsig*OSADA_BLOCK_SIZE;
	inicioDatos= aux;

	tamanioAdmin = header->fs_blocks - header->data_blocks;
	marcarBloque(tamanioAdmin+2, 1);
	marcarBloque(tamanioAdmin+5, 1);

	int j=0;
	for (j=0;j<bitmap.size;j++)
	{
		if (bitmap.bitarray[j]== -1) printf ("11111111 ");
		else if (bitmap.bitarray[j] == 0) printf("00000000 ");
		else printf("%d", bitmap.bitarray[j]);
		if (!((j+1)%4)) printf (" ");
		if (!((j+1)%8)) printf ("\n");

	}


	tablaArchivos[0].state=DIRECTORY;
	memcpy(tablaArchivos[0].fname, "datos\0", 6);
	tablaArchivos[0].parent_directory=0xFFFF;
	tablaArchivos[0].first_block = -1;
	tablaArchivos[2].state=REGULAR;
	memcpy(tablaArchivos[2].fname, "archivo1\0", 9);
	tablaArchivos[2].parent_directory=0;
	tablaArchivos[2].first_block = tamanioAsig+2;
	tablaAsignaciones[tamanioAsig]=-1;
	tablaAsignaciones[tamanioAsig+2]=tamanioAsig+5;
	tablaAsignaciones[tamanioAsig+5]=-1;
	tablaArchivos[10].state=DIRECTORY;
	memcpy(tablaArchivos[10].fname, "Subcarpeta\0", 11);
	tablaArchivos[10].parent_directory=0xFFFF;
	tablaArchivos[10].first_block = -1;
	tablaAsignaciones[tamanioAsig+15]=-1;

	guardarDato("Ma que buonna dona!\0", tamanioAdmin+2);
	memset(inicioDatos[5], 66, OSADA_BLOCK_SIZE);

	listar();

	munmap(header, fsStat.st_size);
	close(fd_fileSystem);

	return 0;
}
