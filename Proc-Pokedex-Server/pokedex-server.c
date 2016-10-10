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
#include "headers/socket.h"












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

	/*
	FILE* f;
	osada_header* header;
	osada_file* file;

	f= fopen ("ejemplo.bin", "rb+");

	leer_archivo_osada(f,header,file);
	*/

	fd_set master;   // conjunto maestro de descriptores de fichero
	fd_set read_fds; // conjunto temporal de descriptores de fichero para select()

	int fdmax;        // número máximo de descriptores de fichero
	int listener;     // descriptor de socket a la escucha
	void* buffer;
	int tam_buffer = 200;
	int nbytes;
	int i, j;
	FD_ZERO(&master);    // borra los conjuntos maestro y temporal
	FD_ZERO(&read_fds);

	int puerto = 9995;
	listener = socket_startListener(puerto);

	// añadir listener al conjunto maestro
	FD_SET(listener, &master);

	// seguir la pista del descriptor de fichero mayor
	fdmax = listener; // por ahora es éste

	// bucle principal
		for (;;) {
			read_fds = master; // cópialo

			//Buscamos los sockets que quieren realizar algo con Select
			socket_select(fdmax, &read_fds);

			// explorar conexiones existentes en busca de datos que leer
			for(i = 0; i <= fdmax; i++) {

				if (FD_ISSET(i, &read_fds)) { // ¡¡tenemos datos!!

					if (i == listener) {
						//ACEPTAMOS UN NUEVO CLIENTE
						//socket_addNewConnection lo que ace
						socket_addNewConection(listener,&master,&fdmax);
					}

					else {

						buffer = malloc(tam_buffer);
						// gestionar datos de un cliente
						if ((nbytes = recv(i, buffer,200, 0)) <= 0) { // error o conexión cerrada por el cliente

							if (nbytes == 0) {
								socket_closeConection(i,&master);

							}
							else {
								perror("recv");
							}

						}

						else
						{
							//El cliente "i" quiere algo


						}
					} // Esto es ¡TAN FEO!
				}
			}
		}

return 0;


}
