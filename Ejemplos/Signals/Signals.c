#include <stdio.h>
#include <signal.h>
#include <stdio.h>
#include <tad_items.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

//BUENOS DIAS, ESTE ES EL EJEMPLO PARA VER QUE CORNO SON LAS SEÑALES, PARA QUE SIRVEN Y QUE MAGIA (DROGA) HAY ATRAS.
//PARA EMPEZAR, UNA SEÑAL ES UNA SYSCALL. DICHA SYSCALL ES UTLIZADA PARA REALIZAR ACCIONES SOBRE PROCESOS ACTIVOS.
//DICHAS SEÑALES SON EN MODO KERNELL ASI QUE VAN A INTERRUMPIR EL PROCESO UNA VEZ QUE SE PRODUSCAN Y PRODUCIR CIERTAS
//ACCIONES A NUESTRO PROCESO.
//COMO NOSOTROS QUEREMOS QUE NUESTRO LINDO PROCESO SE EJECUTE EN PAZ, LO QUE HACEMOS ES ENMASCARAR LAS SEÑALES.
//EN CRIOLLO, ENMASCARAR LA SEÑAL ES HACER QUE UNA SEÑAL SE DISFRASE EN ALGO DISTINTO, COSA DE QUE EL PROCESO, ANTE
//DICHA SEÑAL, PUEDA IGNORARLA O HACER ALGO MAS.

//COMO SE PUEDE HACER ESTA MAGIA? CON signal().
int i;


//Definimos nuestra funcion que atendera las señales entrantes que nosotros queramos manejar.
//En este ejemplo use SIGINT, SIGUSR1, SIGUSR2 Y SIGTERM.QUE ES ESTO?
//Estas varaibles, que no son otra cosa que enteros, son nuestras mascaras, que usaremos con una unica señal, llamada kill.
//cuando enviemos la señal, nostros vamos a "enmascararla" con un numero. Ese numero es el que va a atender nuestra funcion.

//SIGINT = 2
//SIGUSR1 = 10
//SIGUSR2 = 12
//SIGTERM = 15

void manejador(int n){

	switch (n) {
			case SIGINT:
				printf("Okokok, ahora en criollo, me queres matar?\n");
			break;
			case SIGUSR1:
				printf("Parate de manos si sos tan guapo!!\n");
			break;
			case SIGUSR2:
				//EN ESTE CASO, EN LUGAR DE TIRAR UN MENSAJITO PEDORRO, PONEMOS EL CONTEO EN 0.
				i=0;
			break;
			case SIGTERM:
				printf("Terminado... apa, te la creiste\n");
			break;
		}
}

//ACA ESTA LA MAGIA. signal() recibe como parametro un int, que es la mascara que atendera de las señales entrantes
//y una funcion, que en este caso es el manejador, que es la que se encargar de manejar esa señal,
//y va a hacer algo en el momento en que llegue.


//En el momento de que se llega una señal, se interrumpe el flujo del proceso, dejando guardado la ultima instruccion
//ejecutada, se atiende la señal y una vez atendida, se vuelve a donde se habia quedado.
int main(int argc, char** argv){
	i=0;
	signal(SIGTERM, manejador);
	signal(SIGINT, manejador);
	signal(SIGUSR1, manejador);
	signal(SIGUSR2, manejador);
	while(1){
		printf("%i\n", i);
		i++;
		sleep(2);
	}
}

//Pero fede,como corno se usa esto?
//De la siguiente manera:

//1- ejecuta este hermoso proceso
//2- abri otra consola de linux (ctrl+alt+t, para los pro(?))
//3-Para enviar una señal, necesitamos saber la ip del proceso a la que queremos mandarle.
//para eso tiramos el siguiente comando: ps -aux | grep Signals
//4- entre la info que te tira, el primer numero que te tira es la ip de nuestro proceso
//5- Tiramos señales (fijense que va pasando del lado del proceso):
//hacemos en la segunda consola que abrimos esto:
//kill -2 numeroDelProceso
//kill -10 numeroDelProces
//kill -12 numeroDelProces
//kill -15 numeroDelProces

//para el que no se dio cuenta, 2-10-12-15 son las mascaras que establecimos en nuestro proceso que ibamos a atender
//para ver que el resto de las mascaras que no manejamos, realizan sus operaciones por default, tiren losiguiente:
//kill -9 numeroDelProoceso
//como veran, el proceso se termino, porque no establecimos ningun procedimiento que atienda esas señales


//Bueno gente, esto fue signals() para Nintendo64, y espero que les haya gustado, chau.
