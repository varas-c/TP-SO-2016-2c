/*
 * server.c
 *
 *  Created on: 15/8/2016
 *      Author: utnso
 */
/*
 * server.c
 *
 *  Created on: 12/8/2016
 *      Author: utnso
 */
/*
 ** selectserver.c -- servidor de chat multiusuario
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define PORT 9034   // puerto en el que escuchamos


//Obtiene un listener, si hay error, exit(1).
int socket_listener() {

	int listener;

	if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	return listener;
}

void socket_setsockopt(int listener)
{
	int yes = 1;
	// obviar el mensaje "address already in use" (la dirección ya se está usando)
	if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))== -1) {
		perror("setsockopt");
		exit(1);
	}
}

void socket_bind(int listener)
{
	struct sockaddr_in myaddr;     // dirección del servidor
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = INADDR_ANY;
	myaddr.sin_port = htons(PORT);
	memset(&(myaddr.sin_zero), '\0', 8);

	if (bind(listener, (struct sockaddr *) &myaddr, sizeof(myaddr)) == -1) {
		perror("bind");
		exit(1);
	}

}

void socket_listen(int listener)
{
	if (listen(listener, 10) == -1) {
		perror("listen");
		exit(1);
	}
}

void socket_select(int fdmax, fd_set *read_fds)
{
	if (select(fdmax + 1, read_fds, NULL, NULL, NULL) == -1) {
		perror("select");
		exit(1);
	}
}

void socket_addNewConection(int listener, fd_set *master, int *fdmax)
{
	int addrlen;
	struct sockaddr_in remoteaddr; // dirección del cliente
	int newfd;        // descriptor de socket de nueva conexión aceptada

	// gestionar nuevas conexiones
	addrlen = sizeof(remoteaddr);

	if ((newfd = accept(listener,(struct sockaddr *) &remoteaddr, &addrlen)) == -1) {
		perror("accept");
	}

	else {
		FD_SET(newfd, master); // añadir al conjunto maestro

		if (newfd > *fdmax) {    // actualizar el máximo
			*fdmax = newfd;
		}

	printf("selectserver: new connection from %s on ""socket %d\n", inet_ntoa(remoteaddr.sin_addr),newfd);

	}
}

void socket_closeConection(int socket, fd_set *master)
{
	printf("selectserver: socket %d hung up\n", socket);
	close(socket); // bye!
	FD_CLR(socket, master); // eliminar del conjunto maestro
}

int main(void) {
	fd_set master;   // conjunto maestro de descriptores de fichero
	fd_set read_fds; // conjunto temporal de descriptores de fichero para select()

	int fdmax;        // número máximo de descriptores de fichero
	int listener;     // descriptor de socket a la escucha
	void* buf;
	int nbytes;
	int i, j;
	FD_ZERO(&master);    // borra los conjuntos maestro y temporal
	FD_ZERO(&read_fds);

	// obtener socket a la escucha
	listener = socket_listener();

	//setearSocket
	socket_setsockopt(listener);


	// enlazar
	socket_bind(listener);

	// escuchar
	socket_listen(listener);

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
					socket_addNewConection(listener,&master,&fdmax);
				}

				else {

					buf = malloc(200);
					// gestionar datos de un cliente
					if ((nbytes = recv(i, buf,200, 0)) <= 0) { // error o conexión cerrada por el cliente

						if (nbytes == 0) {
							socket_closeConection(i,&master);

						}
						else {
							perror("recv");
						}

					}

					else {

						// tenemos datos de algún cliente
						for (j = 0; j <= fdmax; j++) {
							// ¡enviar a todo el mundo!
							if (FD_ISSET(j, &master)) {
								// excepto al listener y a nosotros mismos
								if (j != listener && j != i) {
									if (send(j, buf, 100, 0) == -1) {
										perror("send");
									}
								}
							}
						}

					}
				} // Esto es ¡TAN FEO!
			}
		}
	}

	return 0;
}

