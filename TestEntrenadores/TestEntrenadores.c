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

//#define MAX_ENTRENADORES 28

int main(int argc, char** argv)
{
	int MAX_ENTRENADORES = 0;

	char* rutaPokedex = strdup(argv[1]);
	MAX_ENTRENADORES = atoi(argv[2]);
	int delay = atoi(argv[3]);

	char** entrenadores;
	entrenadores = malloc(sizeof(char*)*30);
	int i;


	for(i=0;i<MAX_ENTRENADORES;i++)
	{
		entrenadores[i] = malloc(sizeof(char)*50);
	}

	for(i=0;i<MAX_ENTRENADORES;i++)
	{
		switch(i)
		{
			case 0:
				strcpy(entrenadores[i],"Ash");
				break;
			case 1:
				strcpy(entrenadores[i],"Brock");
				break;
			case 2:
				strcpy(entrenadores[i],"Misty");
				break;
			case 3:
				strcpy(entrenadores[i],"Rojo");
				break;
			case 4:
				strcpy(entrenadores[i],"Chris");
				break;
			case 5:
				strcpy(entrenadores[i],"Fede");
				break;
			case 6:
				strcpy(entrenadores[i],"Mati");
				break;
			case 7:
				strcpy(entrenadores[i],"Estefi");
				break;
			case 8:
				strcpy(entrenadores[i],"Seba");
				break;
			case 9:
				strcpy(entrenadores[i],"Red");
				break;
			case 10:
				strcpy(entrenadores[i],"Lance");
				break;
			case 11:
				strcpy(entrenadores[i],"James");
				break;
			case 12:
				strcpy(entrenadores[i],"Jessie");
				break;
			case 13:
				strcpy(entrenadores[i],"Joy");
				break;
			case 14:
				strcpy(entrenadores[i],"May");
				break;
			case 15:
				strcpy(entrenadores[i],"Max");
				break;
			case 16:
				strcpy(entrenadores[i],"Blue");
				break;
			case 17:
				strcpy(entrenadores[i],"Lt. Surge");
				break;
			case 18:
				strcpy(entrenadores[i],"Erika");
				break;
			case 19:
				strcpy(entrenadores[i],"Sabrina");
				break;
			case 20:
				strcpy(entrenadores[i],"Koga");
				break;
			case 21:
				strcpy(entrenadores[i],"Blaine");
				break;
			case 22:
				strcpy(entrenadores[i],"Giovanni");
				break;
			case 23:
				strcpy(entrenadores[i],"Panchito");
				break;
			case 24:
				strcpy(entrenadores[i],"Gary");
				break;
			case 25:
				strcpy(entrenadores[i],"Norman");
				break;
			case 26:
				strcpy(entrenadores[i],"Winona");
				break;
			case 27:
				strcpy(entrenadores[i],"Volkner");
				break;
		}

	}

	for(i=0;i<MAX_ENTRENADORES;i++)
	{
		printf("%s \n",entrenadores[i]);
	}


	char** vector;
	vector = malloc(sizeof(char*)*28);


	int cantMaxEntrenadores = 28;
	int j;

	for(j=0;j<cantMaxEntrenadores;j++)
	{
		vector[j] = malloc(sizeof(char)*512);
		sprintf(vector[j],"xterm -hold -e ./Proc-Entrenador %s %s", rutaPokedex, entrenadores[j]);
	}

	for(j=0;j<cantMaxEntrenadores;j++)
	{
		printf("%s \n", vector[j]);
	}



	int pid;

	for(i=0;i<MAX_ENTRENADORES;i++)
	{
		if(pid = fork() == 0){
			sleep(delay);
			system(vector[i]);
			break;
		}

	}

	while(1) sleep(1);

    return 0;
}

