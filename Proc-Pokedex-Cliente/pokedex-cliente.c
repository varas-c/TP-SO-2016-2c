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
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>
#include <commons/string.h>
#include <commons/config.h>
#include "headers/struct.h"
#include "headers/socket.h"
#include "headers/osada.h"

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

//enum codigo_operacion {GETATTR, READDIR, READ, TRUNCATE, WRITE};
//ATENCIÓN: off_t ocupa 8 bytes acá, y 4 en el servidor. Tener en cuenta al hacer sizeof(off_t)

char* numero_IP = "127.0.0.1";
char* numero_Puerto = "10000";
int fd_server;

//Wrappers de las funciones de sockets para contemplar envíos parciales

uint8_t recibir(int socket, void* buffer, uint64_t total)
{
	uint64_t parcial=0;
	do
	{
		if((parcial+= recv(socket, buffer+parcial, total-parcial, 0))<=0)
			return 0;
	}while(parcial<total);
	return 1;
}

uint8_t enviar(int socket, void* buffer, uint64_t total)
{
	uint64_t parcial=0;
	do
	{
		if((parcial+= send(socket, buffer+parcial, total-parcial, 0))<=0)
			return 0;
	}while(parcial<total);
	return 1;
}

void finalizarProceso(int signal)
{
	if(signal==SIGTERM || signal==SIGINT || signal == SIGHUP)
		close(fd_server);
	exit(0);
}

void enviarCodigoYPath(uint8_t codigo, const char* path)
{
	void* buffer;
	buffer = malloc(2+strlen(path)+1);
	memset(buffer, codigo, 1);
	memset(buffer+1, strlen(path)+1, 1);

	memcpy(buffer+2, path, strlen(path)+1);
	enviar(fd_server, buffer, 2+strlen(path)+1);
	free(buffer);
	return;
}


void enviarMensaje(char* mensaje)
{
	void* buffer;
	buffer =malloc(strlen(mensaje)+2);
	memset(buffer,0,strlen(mensaje)+2);

	memset(buffer,strlen(mensaje)+1,1);
	memcpy((char*)buffer+1,mensaje,strlen(mensaje));

	send(fd_server,buffer,strlen(mensaje)+2,0);

	free(buffer);
	return;

}


static int osada_getattr(const char *path, struct stat *stbuf)
{
	void* buffer;
	int res = 0;
	int retorno = 0;

	memset(stbuf, 0, sizeof(struct stat));

	if (strcmp(path, "/") == 0)
	{
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	}
	else
	{
		enviarCodigoYPath(COD_GETATTR, path);
		buffer = malloc(1);
		if ((res = recv(fd_server, buffer, 1, 0)) <= 0)
		{
			printf("El servidor se encuentra desconectado.\n");
			free(buffer);
			return -ENOENT;
		}
		if (!*(uint8_t*)buffer)
			retorno = -ENOENT;
		else
		{
			free(buffer);
			buffer = malloc(sizeof(osada_file));
			if (!recibir(fd_server, (osada_file*)buffer, sizeof(osada_file)))
			{
				printf("El servidor se encuentra desconectado.\n");
				retorno = -ENOENT;
			}
			else
			{
				osada_file* archivo = (osada_file*) buffer;
				if (archivo->state== DIRECTORY)
				{
					stbuf->st_mode = S_IFDIR | 0755;
					stbuf->st_nlink = 2;
					stbuf->st_mtim.tv_sec = (__time_t)(archivo->lastmod);
				}
				else if (archivo->state == REGULAR)
				{
					stbuf->st_mode = S_IFREG | 0777;
					stbuf->st_nlink = 1;
					stbuf->st_size = archivo->file_size;
					stbuf->st_mtim.tv_sec = (__time_t)(archivo->lastmod);
				}
				else
					retorno = -ENOENT;
			}
		}
		free(buffer);
	}
	return retorno;
}

static int osada_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
	void* buffer;
	int retorno = 0;
	int res=0;

	enviarCodigoYPath(COD_READDIR, path);
	buffer = malloc(1);
	if ((res = recv(fd_server, buffer, 1, 0))<=0)
	{
		printf("El servidor se encuentra desconectado.\n");
		free(buffer);
		return -ENOENT;
	}

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	if ((signed char) *(signed char*)buffer==1)
	{
		do
		{
			free(buffer);
			buffer = malloc(OSADA_FILENAME_LENGTH+1);
			memset(buffer, 0, OSADA_FILENAME_LENGTH+1);
			if (!recibir(fd_server, buffer, OSADA_FILENAME_LENGTH+1))
			{
				printf("El servidor se encuentra desconectado.\n");
				retorno = -ENOENT;
			}
			if (strlen(buffer)>0)
				filler(buf, buffer, NULL, 0);
		}while((strlen(buffer)>0) && (retorno!=-ENOENT));
	}
	else if ((signed char) *(signed char*)buffer==-1)
		retorno = -ENOENT;


	free(buffer);
	return retorno;
}

static size_t osada_read(const char *path, char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi) {

	void* buffer;
	size_t retorno = size;
	int res=0;

	//ATENCIÓN: off_t ocupa 8 bytes acá, y 4 en el servidor. Tener en cuenta al hacer sizeof(off_t)

	enviarCodigoYPath(COD_READ, path);
	buffer = malloc(sizeof(size)+sizeof(offset));
	memcpy(buffer, &size, sizeof(size));
	memcpy(buffer+sizeof(size), &offset, sizeof(offset));
	send(fd_server, buffer, sizeof(size)+sizeof(offset), 0);
	free(buffer);

	if ((res = recv(fd_server, &retorno, sizeof(retorno), 0))<=0)
	{
		printf("El servidor se encuentra desconectado.\n");
		retorno = 0;
	}

	if (retorno>0)
	{
		buffer = malloc(retorno);
		if (!recibir(fd_server, buffer, retorno))
		{
			printf("El servidor se encuentra desconectado.\n");
			retorno = 0;
		}
		memcpy(buf, buffer, retorno);
		free(buffer);
	}
	else
		memset(buf, 0, retorno);

	return retorno;
}

int osada_truncate(const char * path, off_t size) //Truncate debe estar para que write funcione
{
	void* buffer;
	uint8_t retorno=0;

	enviarCodigoYPath(COD_TRUNCATE, path);
	buffer = malloc(sizeof(size));
	memcpy(buffer, &size, sizeof(size));
	send(fd_server, buffer, sizeof(size), 0);
	free(buffer);
	if (recv(fd_server, &retorno, sizeof(retorno), 0)<=0)
	{
		printf("El servidor se encuentra desconectado.\n");
		return 0;
	}

	if (!retorno)
		return -ENOSPC;
	return 0;
}

static int osada_write(const char *path, const char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi) {

	void* buffer;
	int retorno = size;
	int res=0;

	enviarCodigoYPath(COD_WRITE, path);

	buffer=malloc(sizeof(size_t)+sizeof(off_t)+size);
	memcpy(buffer, &size, sizeof(size));
	memcpy(buffer+sizeof(size_t), &offset, sizeof(off_t));
	memcpy(buffer+sizeof(size_t)+sizeof(off_t), buf, size);
	enviar(fd_server, buffer, sizeof(size_t)+sizeof(off_t)+size);
	free(buffer);
	buffer = malloc(1);
	if ((res = recv(fd_server, buffer, 1, 0))<=0)
	{
		printf("El servidor se encuentra desconectado.\n");
		retorno = -1;
	}
	if (!(((uint8_t*)buffer)[0]))
		retorno = -ENOSPC;

	free(buffer);
	return retorno;
}

int osada_create (const char *path, mode_t tipo, struct fuse_file_info *fi)
{
	void* buffer;
	int retorno = 0;
	int res=0;

	enviarCodigoYPath(COD_CREATE, path);

	buffer = malloc(1);
	if ((res = recv(fd_server, buffer, 1, 0))<=0)
	{
		printf("El servidor se encuentra desconectado.\n");
		retorno = -1;
	}

	switch(((uint8_t*)buffer)[0])
	{
	case 1: retorno = -ENOENT; break;
	case 2: retorno = -ENAMETOOLONG; break;
	case 3: retorno = -EEXIST; break;
	case 4: retorno = -ENOSPC; break;
	}

	free(buffer);
	return retorno;
}

int osada_mkdir(const char* path, mode_t mode)
{
	void* buffer;
	int retorno = 0;
	int res=0;

	enviarCodigoYPath(COD_MKDIR, path);

	buffer = malloc(1);
	if ((res = recv(fd_server, buffer, 1, 0))<=0)
	{
		printf("El servidor se encuentra desconectado.\n");
		retorno = -1;
	}

	switch(((uint8_t*)buffer)[0])
	{
	case 1: retorno = -ENOENT; break;
	case 2: retorno = -ENAMETOOLONG; break;
	case 3: retorno = -EEXIST; break;
	case 4: retorno = -ENOSPC; break;
	}

	free(buffer);
	return retorno;
}

int osada_rmdir(const char* path)
{
	void* buffer;
	int retorno = 0;
	int res=0;

	enviarCodigoYPath(COD_RMDIR, path);
	buffer = malloc(1);
	if ((res = recv(fd_server, buffer, 1, 0))<=0)
	{
		printf("El servidor se encuentra desconectado.\n");
		retorno = -1;
	}

	switch(((uint8_t*)buffer)[0])
	{
	case 1: retorno = -ENOENT; break;
	case 2: retorno = -ENOTEMPTY; break;
	}

	free(buffer);
	return retorno;
}

int osada_unlink(const char* path)
{
	void* buffer;
	int retorno = 0;
	int res=0;

	enviarCodigoYPath(COD_UNLINK, path);
	buffer = malloc(1);
	if ((res = recv(fd_server, buffer, 1, 0))<=0)
	{
		printf("El servidor se encuentra desconectado.\n");
		retorno = -1;
	}

	if(!((uint8_t*)buffer)[0])
		retorno = -ENOENT;

	free(buffer);
	return retorno;
}


int osada_rename (const char *path,char* nuevoPath)
{
	void* buffer;
	int retorno = 0;
	int res=0;

	enviarCodigoYPath(COD_RENAME, path);
	enviarMensaje(nuevoPath);

	buffer = malloc(1);
	if ((res = recv(fd_server, buffer, 1, 0))<=0)
	{
		printf("El servidor se encuentra desconectado.\n");
		retorno = -1;
	}

	switch(((uint8_t*)buffer)[0])
	{
	case 1: retorno = -ENOENT; break;
	case 2: retorno = -ENAMETOOLONG; break;
	case 3: retorno = -EEXIST; break;
	}

	free(buffer);
	return retorno;
}

static struct fuse_operations osada_oper = {
		.getattr = osada_getattr,
		.readdir = osada_readdir,
		.read = osada_read,
		.truncate = osada_truncate,
		.write = osada_write,
		.create = osada_create,
		.mkdir = osada_mkdir,
		.rmdir = osada_rmdir,
		.unlink = osada_unlink,
		.rename = osada_rename,
};


int main(int argc, char* argv[])
{
	numero_IP = getenv("POKE_SERVER_IP");
	numero_Puerto = getenv("POKE_SERVER_PUERTO");
	fd_server = get_fdServer(numero_IP,numero_Puerto); //el fd_server es el "socket" que necesitas para comunicarte con el mapa
/*
// Uso de señales. Por ahora no hacen falta, el server detecta la desconexion
	signal(SIGINT, finalizarProceso);
	signal(SIGTERM, finalizarProceso);
	signal(SIGHUP, finalizarProceso);
*/
	return fuse_main(argc, argv, &osada_oper, NULL);

	return 0;
}
