/*
 * socket.h
 *
 *  Created on: 30/8/2016
 *      Author: utnso
 */

#ifndef HEADERS_SOCKET_H_
#define HEADERS_SOCKET_H_

#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int fd_server;
int vidas_restantes;

int get_fdServer(char* numero_IP, char* numero_Puerto)
{
			struct addrinfo hints;			// Estructuras de libreria para guardar info de conexion
			struct addrinfo *serverInfo;

			memset(&hints, 0, sizeof(hints));	// Inicializa todas las variables de la estructura en 0, para evitar errores por contener basura
			hints.ai_family = AF_UNSPEC;		// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
			hints.ai_socktype = SOCK_STREAM;	// Indica que usamos Socket Stream


			//valor_getaddrinfo solo sirve para ver si se pudo obtener info del server, despues no sirve para nada
			int valor_getaddrinfo = getaddrinfo(numero_IP, numero_Puerto, &hints, &serverInfo);		// Carga en serverInfo los datos de la conexion

		    if (valor_getaddrinfo != 0) {
		        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(valor_getaddrinfo));
		        exit(1);
		    }

			int serverSocket;		// File Descriptor del socket
			serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);	//Inicializamos el valor del serverSocket

			if(serverSocket == -1) {
				printf("Error de Socket Servidor");
				exit(1);
			} //Error de conexion.

			int valor_connect = connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);		// Manda una peticion de conexion a donde apunta el socket


			if(valor_connect == -1) {
				printf("Error de Conexion");
				exit(1);
			}

			if (serverInfo == NULL) {
			        fprintf(stderr, "client: failed to connect\n");
			        exit(1);
			    }

			freeaddrinfo(serverInfo);	// No lo necesitamos mas

			return serverSocket;
}
//******************************************

void manejar_signals(int operacion){

	if(flag_SIGNALMUERTE == false)
	{
		fflush(stdout);
		printf("\n\nSEÑAL SEÑAL\n\n");
		switch(operacion){

		case SIGUSR1: //Le sumamos una vida al entrenador
			entrenador.vidas += 1;
			break;
		case SIGTERM://Le restamos uan vida al entrenador
			entrenador.vidas -= 1;

			if(entrenador.vidas == 0)
			{
				flag_SIGNALMUERTE = true;

				if(flag_BLOQUEADO) close(fd_server);
			}
			break;
		}
	}
}
//******************************************



#endif /* HEADERS_SOCKET_H_ */
