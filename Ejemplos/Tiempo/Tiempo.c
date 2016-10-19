#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/config.h>
#include <ctype.h>
#include <time.h>

typedef struct{
	int minutos;
	int segundos;
}tiempo;

tiempo tiempoTardado(tiempo inicio, tiempo fin){
	int totalInicio, totalFin, tardadoEnSegundos;
	tiempo tiempoTardado;
	totalInicio = ((inicio.minutos)*60 + (inicio.segundos));
	totalFin = ((fin.minutos)*60 + (fin.segundos));
	tardadoEnSegundos = totalFin - totalInicio;
	tiempoTardado.minutos = (tardadoEnSegundos / 60);
	tiempoTardado.segundos = (tardadoEnSegundos % 60);
	return tiempoTardado;
}

int main(){
	struct tm *local, *local2;
	time_t t, t2;
	tiempo tardado,inicio,fin;
	t = time(NULL);
	local = localtime(&t);
	inicio.minutos = local->tm_min;
	inicio.segundos = local->tm_sec;
	printf("La hora es: %d : %d : %d\n", local->tm_hour, local->tm_min, local->tm_sec);
	sleep(156);
	t2 = time(NULL);
	local2 = localtime(&t2);
	fin.minutos = local2->tm_min;
	fin.segundos = local2->tm_sec;
	printf("La hora es: %d : %d : %d\n", local2->tm_hour, local2->tm_min, local2->tm_sec);
	tardado = tiempoTardado(inicio, fin);
	printf("Se tardo: %d minutos y %d segundos", tardado.minutos, tardado.segundos);
	getchar();
	return 0;

}
