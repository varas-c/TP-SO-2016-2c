/*
 * socket.h
 *
 *  Created on: 30/8/2016
 *      Author: utnso
 */

#ifndef HEADERS_SOCKET_H_
#define HEADERS_SOCKET_H_
#define PORT 9034

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "struct.h"

//Obtiene un listener, si hay error, exit(1).
int socket_listener()
{	int listener;
	if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}
	return listener;
}

void socket_setsockopt(int listener)
{	int yes = 1;
	// obviar el mensaje "address already in use" (la dirección ya se está usando)
	if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))== -1) {
		perror("setsockopt");
		exit(1);
	}
}
//****************************************************************************************************************

void socket_bind(int listener, int port)
{	struct sockaddr_in myaddr;     // dirección del servidor
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = INADDR_ANY;
	myaddr.sin_port = htons(port);
	memset(&(myaddr.sin_zero), '\0', 8);

	if (bind(listener, (struct sockaddr *) &myaddr, sizeof(myaddr)) == -1) {
		perror("bind");
		exit(1);
	}
}
//****************************************************************************************************************

void socket_listen(int listener)
{	if (listen(listener, 10) == -1) {
		perror("listen");
		exit(1);
	}
}
//****************************************************************************************************************



void socket_select(int fdmax, fd_set *read_fds)
{

	tv.tv_sec = 1;
	tv.tv_usec = 0;

	while(1)
	{

	if (select(fdmax + 1, read_fds, NULL, NULL, &tv) < 0)
	{
		if(errno == EINTR)
		{
			 printf("Señal recibida");
		}

		else
		{
			perror("Select Error");
			exit(1);
		}
	}

	else
	{
		return;
	}

	}

}
//****************************************************************************************************************

int socket_addNewConection(int listener, fd_set *master, int *fdmax)
{	int addrlen;
	struct sockaddr_in remoteaddr; // dirección del cliente
	int newfd;        // descriptor de socket de nueva conexión aceptada
	// gestionar nuevas conexiones
	addrlen = sizeof(remoteaddr);

	if ((newfd = accept(listener,(struct sockaddr *) &remoteaddr, &addrlen)) == -1) {
		perror("accept");
	}

	else {
		/*
		FD_SET(newfd, master); // añadir al conjunto maestro
		*/

		if (newfd > *fdmax) {    // actualizar el máximo
			*fdmax = newfd;
		}

	//printf("selectserver: new connection from %s on ""socket %d\n", inet_ntoa(remoteaddr.sin_addr),newfd);
	}
	return newfd;
}
//****************************************************************************************************************

void socket_closeConection(int socket, fd_set *master)
{
	printf("selectserver: socket %d hung up\n", socket);
	close(socket); // bye!
	FD_CLR(socket, master); // eliminar del conjunto maestro
}
//****************************************************************************************************************

int socket_startListener(puerto)
{
	int listener;
	// obtener socket a la escucha
	listener = socket_listener();

	//setearSocket
	socket_setsockopt(listener);

	// enlazar
	socket_bind(listener, puerto);

	// escuchar
	socket_listen(listener);

	return listener;
}

#endif /* HEADERS_SOCKET_H_ */
