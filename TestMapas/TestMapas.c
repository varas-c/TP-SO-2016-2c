/*
 * TestMapas.c
 *
 *  Created on: 7/10/2016
 *      Author: utnso
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main (int argc, char *argv[])
{
	/*
	signal(SIGTERM, manejar_signals);
	signal(SIGHUP, manejar_signals);
	signal(SIGINT, manejar_signals);

	*/

	char** vector;
	vector = malloc(sizeof(char*)*8);

	int j;
	for(j=0;j<=7;j++)
	{
		vector[j] = malloc(sizeof(char)*256);
	}

	strcpy(vector[0],"xterm -e ./Proc-Mapa ../..//Proc-Pokedex-Cliente/montaje/pokedex CiudadAzafran");
	strcpy(vector[1],"xterm -e ./Proc-Mapa ../../Proc-Pokedex-Cliente/montaje/pokedex CiudadCeleste");
	strcpy(vector[2],"xterm -e ./Proc-Mapa ../../Proc-Pokedex-Cliente/montaje/pokedex CiudadFucsia");
	strcpy(vector[3],"xterm -e ./Proc-Mapa ../../Proc-Pokedex-Cliente/montaje/pokedex CiudadPlateada");
	strcpy(vector[4],"xterm -e ./Proc-Mapa ../../Proc-Pokedex-Cliente/montaje/pokedex CiudadVerde");
	strcpy(vector[5],"xterm -e ./Proc-Mapa ../../Proc-Pokedex-Cliente/montaje/pokedex IslaCanela");
	strcpy(vector[6],"xterm -e ./Proc-Mapa ../../Proc-Pokedex-Cliente/montaje/pokedex PuebloLavanda");
	strcpy(vector[7],"xterm -e ./Proc-Mapa ../../Proc-Pokedex-Cliente/montaje/pokedex PuebloPaleta");



	int i;
	int pid;

	for(i=0;i<=7;i++)
	{
		if(pid = fork() == 0){
			system(vector[i]);
			break;
		}

	}

	while(1) sleep(1);



/*
	int i=0;
	int pid;

	for(i=0;i<5;i++)
	{


		if(pid = fork() == 0)
		{
			printf("PID %i",getppid());
			break;
		}
	}
*/




    return 0;
}
