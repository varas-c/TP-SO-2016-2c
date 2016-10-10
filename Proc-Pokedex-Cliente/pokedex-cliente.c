/*
 * Pokedex-Cliente.c
 *
 *  Created on: 30/8/2016
 *      Author: utnso
 */

#include <stdio.h>
#include <commons/config.h>
#include "headers/struct.h"
#include "headers/socket.h"

int main(int argc, char** argv)
{
	char* numero_IP = "127.0.0.1";
	int numero_Puerto = 9995;
	int fd_server = get_fdServer(numero_IP,numero_Puerto); //el fd_server es el "socket" que necesitas para comunicarte con el mapa


	return 0;
}
