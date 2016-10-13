/*
 * Pokedex-Cliente.c
 *
 *  Created on: 30/8/2016
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fuse.h>
#include <stddef.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <commons/string.h>
#include <commons/config.h>
#include "headers/struct.h"
#include "headers/socket.h"
#include "headers/osada.h"

char* numero_IP = "127.0.0.1";
char* numero_Puerto = "9995";
int fd_server;

void finalizarProceso(int signal)
{
	//if(signal==SIGTERM || signal==SIGINT || signal == SIGHUP)
	close(fd_server);
	exit(0);
}

void enviarCodigoyTamanio(int codigo, uint8_t tamanio)
{
	void* buffer;
	buffer = malloc(2);
	memset(buffer, codigo, 1);
	memset(buffer+1, tamanio, 1);
	send(fd_server, buffer, 2, 0);
	free(buffer);
	return;
}

void enviarPath(const char* path)
{
	void* buffer;
	buffer = malloc(strlen(path)+1);
	memcpy(buffer, path, strlen(path)+1);
	printf("Path: %s, buffer: %s\n\n", path, (char*)buffer);
	send(fd_server, buffer, strlen(path)+1, 0);
	free(buffer);
	return;
}

static int osada_getattr(const char *path, struct stat *stbuf)
{
	printf("Entra en getattr\n\n");
	void* buffer;
	int res = 0;
	int retorno = 0;

	memset(stbuf, 0, sizeof(struct stat));

	if (strcmp(path, "/") == 0)
	{
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		stbuf->st_mtim.tv_sec = (__time_t)time(NULL);
	}
	else
	{
		enviarCodigoyTamanio(1, strlen(path)+1);
		enviarPath(path);
		buffer = malloc(32);
		if ((res = recv(fd_server, (osada_file*)buffer, 32, 0)) <= 0)
		{
			fflush(stdout);
			printf("El servidor se encuentra desconectado.\n");
			retorno = -ENOENT;
		}
		else
		{
			osada_file* archivo = (osada_file*) buffer;
			if ( archivo->state== DIRECTORY)
			{
				stbuf->st_mode = S_IFDIR | 0755;
				stbuf->st_nlink = 2;
				stbuf->st_mtim.tv_sec = (__time_t)(archivo->lastmod);
			}
			else if (archivo->state == REGULAR)
			{
				stbuf->st_mode = S_IFREG | 0777;
				stbuf->st_nlink = 1;
				stbuf->st_size = (__off_t)(archivo->file_size);
				stbuf->st_mtim.tv_sec = (__time_t)(archivo->lastmod);
			}
			else
				retorno = -ENOENT;
		}
		free(buffer);
	}

	return retorno;
}

static int osada_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
	printf("Entra en readdir\n\n");
	void* buffer;
	int retorno = 0;
	int res=0;

	enviarCodigoyTamanio(2, strlen(path)+1);
	enviarPath(path);
	buffer = malloc(1);
	if ((res = recv(fd_server, buffer, 1, 0))<=0)
	{
		fflush(stdout);
		printf("El servidor se encuentra desconectado.\n");
		retorno = -ENOENT;
	}

	filler(buf, ".", NULL,0);
	filler(buf, "..", NULL,0);

	if ((signed char) *(signed char*)buffer==1)
	{
		do
		{
			free(buffer);
			buffer = malloc(17);
			memset(buffer, 0, 17);
			if ((res = recv(fd_server, buffer, 17, 0))<=0)
			{
				fflush(stdout);
				printf("El servidor se encuentra desconectado.\n");
				retorno = -ENOENT;
			}
			filler(buf, buffer, NULL, 0);
		}while(strlen(buffer)>0);
	}
	else if ((signed char) *(signed char*)buffer==-1)
		retorno = -ENOENT;


	free(buffer);
	return retorno;
}

static int osada_read(const char *path, char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi) {

	void* buffer;
	int retorno = size;
	int res=0;
	printf("Entra a read, size: %d offset: %d\n\n", size, offset);
	enviarCodigoyTamanio(3, strlen(path)+1);
	enviarPath(path);
	buffer = malloc(sizeof(size)+sizeof(offset));
	memcpy(buffer, &size, sizeof(size));
	memcpy(buffer+sizeof(size), &offset, sizeof(offset));
	send(fd_server, buffer, sizeof(size)+sizeof(offset), 0);
	free(buffer);
	buffer = malloc(size);
	if ((res = recv(fd_server, buffer, size, 0))<=0)
	{
		printf("El servidor se encuentra desconectado.\n");
		retorno = 0;
	}
	memcpy(buf, buffer, size);
	if (!strlen((char*)buffer))
		retorno = 0;
	free(buffer);

	printf("Sale de read. Leyo: %d\n\n", retorno);
	return retorno;
}

static struct fuse_operations osada_oper = {
		.getattr = osada_getattr,
		.readdir = osada_readdir,
		.read = osada_read,
};


int main(int argc, char** argv)
{
	fd_server = get_fdServer(numero_IP,numero_Puerto); //el fd_server es el "socket" que necesitas para comunicarte con el mapa
/*
 * Uso de señales. Por ahora no hacen falta, el server detecta la desconexion
	signal(SIGINT, finalizarProceso);
	signal(SIGTERM, finalizarProceso);
	signal(SIGHUP, finalizarProceso);
*/
	return fuse_main(argc, argv, &osada_oper, NULL);

	return 0;
}
