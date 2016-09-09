/*
 * Pokedex-Server.c
 *
 *  Created on: 30/8/2016
 *      Author: utnso
 */

#include <stdint.h>
#include <stdio.h>
#include <commons/config.h>
#include <commons/log.h>
#include "headers/struct.h"
#include "headers/osada.h"

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

int main(int argc, char** argv)
{
	FILE* f;
	osada_header* header;
	osada_file* file;

	f= fopen ("ejemplo.bin", "rb+");

	leer_archivo_osada(f,header,file);
}
