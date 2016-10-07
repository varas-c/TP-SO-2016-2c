/*
 * TestEntrenadores.c
 *
 *  Created on: 7/10/2016
 *      Author: utnso
 */


/*
 * testingEntrenadores.c
 *
 *  Created on: 7/10/2016
 *      Author: utnso
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char** argv)
{

	int cantEntrenadores = atoi(argv[1]);
	int delay = atoi(argv[2]);


	char** vector;
	vector = malloc(sizeof(char*)*4);


	int cantMaxEntrenadores = 4;
	int j;

	for(j=0;j<cantMaxEntrenadores;j++)
	{
		vector[j] = malloc(sizeof(char)*256);
	}

	strcpy(vector[0],"xterm -e ./Proc-Entrenador /mnt/pokedex Ash");
	strcpy(vector[1],"xterm -e ./Proc-Entrenador /mnt/pokedex Brock");
	strcpy(vector[2],"xterm -e ./Proc-Entrenador /mnt/pokedex Misty");
	strcpy(vector[3],"xterm -e ./Proc-Entrenador /mnt/pokedex Rojo");

	int i;
	int pid;

	for(i=0;i<cantEntrenadores;i++)
	{
		if(pid = fork() == 0){
			system(delay);
			system(vector[i]);
			break;
		}

	}

	while(1) sleep(1);

    return 0;
}

