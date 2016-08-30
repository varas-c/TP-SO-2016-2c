/*

** client.c

*/

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>


#define PORT "9034" // the port client will be connecting to
#define MAXDATASIZE 1024 //Tamaño maximo del paquete en Kb

typedef struct
{
	char usuario[20];
	char mensaje[50];

}Paquete;


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
		    }

			int serverSocket;		// File Descriptor del socket
			serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);	//Inicializamos el valor del serverSocket

			if(serverSocket == -1)
				printf("Error de Socket Servidor"); //Error de conexion.

			int valor_connect = connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);		// Manda una peticion de conexion a donde apunta el socket


			if(valor_connect == -1) {
				printf("Error de Coneccion");
			}

			if (serverInfo == NULL) {
			        fprintf(stderr, "client: failed to connect\n");
			    }

			freeaddrinfo(serverInfo);	// No lo necesitamos mas

			return serverSocket;
}

//La longitud del usuario es 19 como maximo + 1 del /n

int armar_paquete(Paquete *paquete, void** buffer)
{
	int long_usuario = strlen(paquete->usuario) + 1;
	int long_mensaje = strlen(paquete->mensaje) + 1;

	int tam_buffer = sizeof(int)+long_usuario+sizeof(int)+long_mensaje;


	*buffer = malloc(tam_buffer);

	memcpy(*buffer,&long_usuario,sizeof(int));

	memcpy(*buffer+sizeof(int),paquete->usuario,long_usuario);

	memcpy(*buffer+sizeof(int)+long_usuario,&long_mensaje,sizeof(int));

	memcpy(*buffer+sizeof(int)+long_usuario+sizeof(int),paquete->mensaje,long_mensaje);


	return tam_buffer;

}
////////Procedimiento estandar para conectarse a un server/////////
/*Recibe como parametros un char que contiene el mensaje a enviar, el numero de IP del server,el numero de puerto y el tamaño del paquete a enviar*/
void Enviar_Mensaje(char* mensajeAEnviar, int tamanio_Paquete, int fd_server)
{
		char mensaje[tamanio_Paquete];	// Una variable donde almacenar lo que vamos a mandar(codigo)
		strcpy(mensaje, mensajeAEnviar);
		send(fd_server, mensaje, strlen(mensaje) + 1, 0);	// Enviamos el mensaje

		//Aca se recibe un mensaje del servidor SI ES NECESARIO.

		char paquete[tamanio_Paquete];								//	Una variable para almacenar lo recibido(una vez que se establece la conexion se pueden enviar y recibir cosas, en este caso un mensaje de confirmacion de que lo enviado llego)
		int valor_recv = recv(fd_server, (void*) paquete, tamanio_Paquete, 0);		// Se almacena en package, el mensaje recibido
		if(valor_recv == -1) printf("Error recv");
		printf("%s", paquete);		// Muestra en pantalla el mensaje recibido
		close(fd_server);		// Terminamos la comunicacion, por lo tanto cerramos la conexion
}

void quitarEnter(char* cadena)
{
	int i;
	for(i=0;i<50;i++)
	{
		if(cadena[i] == '\n' )
			cadena[i] = '\0';
	}

}

void *thread_Enviar_Mensaje(void *ptr)
{
		Paquete paquete;
		int fd_server = (int) ptr;

		void *buffer;
		int tam_buffer;

		printf("Ingrese su nombre de usuario: ");
		fgets(paquete.usuario,sizeof(paquete.mensaje),stdin);
		quitarEnter(paquete.usuario);

		while(1) {
		fgets(paquete.mensaje, sizeof(paquete.mensaje), stdin);
		quitarEnter(paquete.mensaje);
		tam_buffer = armar_paquete(&paquete,&buffer);

		send(fd_server, buffer, tam_buffer,0);	// Enviamos el mensaje

		free(buffer);
		}

}


Paquete desarmar_paquete(void *buffer_lectura)
{
	Paquete paquete;
	int long_usuario;
	int long_mensaje;

	memcpy(&long_usuario,buffer_lectura,sizeof(int));

	//Ya tengo la longitud del usuario, ahora la leo de la cadena
	memcpy( &paquete.usuario, buffer_lectura+sizeof(int),long_usuario);

	//Ya lei la longitud del usuario y el usuario en si
	//Ahora leo long_mensaje y mensaje

	memcpy(&long_mensaje,buffer_lectura+sizeof(int)+long_usuario,sizeof(int));
	memcpy(&(paquete.mensaje),buffer_lectura+sizeof(int)+long_usuario+sizeof(int),long_mensaje);

	return paquete;
}

void* thread_Recibir_Mensaje(void *ptr)
{
	Paquete paquete;
	void* buffer;
	int tam_buffer = 150;

	int fd_server = (int) ptr;//
	int valor_recv;

	while(1) {
	buffer = malloc(tam_buffer);
	valor_recv = recv(fd_server, buffer, tam_buffer, 0);		// Se almacena en package, el mensaje recibido


	if(valor_recv == -1)  {
		printf("Error recv");
		}

		paquete = desarmar_paquete(buffer);

		fflush(stdout);
		printf("%s : -- %s \n", &(paquete.usuario),&(paquete.mensaje));		// Muestra en pantalla el mensaje recibido
		free(buffer);
	}

}




int main(int argc, char *argv[])
{

	pthread_t thread1, thread2;
	char* numero_IP = "127.0.0.1";
	char* numero_Puerto="9034";

	int fd_server = get_fdServer(numero_IP,numero_Puerto);

	int iret1;
	int iret2;

	iret1 = pthread_create(&thread1,NULL,thread_Enviar_Mensaje,(void*) fd_server);
	iret2 =  pthread_create(&thread1,NULL,thread_Recibir_Mensaje,(void*) fd_server);

	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);

	return 0;

}
