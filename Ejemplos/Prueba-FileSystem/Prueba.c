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

int main()
{
	FILE* fileSystem = NULL;
	fileSystem = fopen("/home/utnso/workspace/Prueba-FileSystem/prueba.bin", "r");
	osada_header header;

	fread(&header, sizeof(header), 1, fileSystem);

	int tamanioBitmap = OSADA_BLOCK_SIZE * header.bitmap_blocks;
	uint8_t bitmap[tamanioBitmap];
	fread(&bitmap, sizeof(bitmap), 1, fileSystem);

	osada_file tablaArchivos[2048];

	fread(&tablaArchivos, sizeof(tablaArchivos), 1, fileSystem);

	int tablaAsignaciones[header.data_blocks];
	fread(&tablaAsignaciones, sizeof(tablaAsignaciones), 1, fileSystem);

	fclose(fileSystem);

	return 0;
}
