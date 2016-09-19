/*
 * FUSE.c
 *
 *  Created on: 19/9/2016
 *      Author: utnso
 */

/* Este es un programa de prueba para intentar entender cómo funciona FUSE.
 * Los comentarios muestran lo que se sabe hasta el momento.
 *
 * Antes que nada, es importante destacar que Linux considera que cualquier cosa es un archivo (incluso los
 * directorios), asi que cuando se mencione la palabra "archivo" no se refiere al archivo de datos
 * tradicional (archivo simple).
 *
 * Al ejecutar desde consola: ./FUSE "carpeta punto de montaje" -f (recomendable) -d (opcional)
 * -f: ejecuta en primer plano, por lo que la consola quda disponible para enviar señales.
 * -d: muestra las operaciones realizadas por FUSE. Puede ser útil para probar cosas.
 * La carpeta punto de montaje debe estar creada, y puede pasarse como ruta relativa.
 *
 * Revisar repo de github para saber qué linkear: https://github.com/sisoputnfrba/so-fuse_example*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fuse.h>
#include <stddef.h>
#include <errno.h>
#include <fcntl.h>

#define DEFAULT_FILE_CONTENT "Hello World!\n"
#define DEFAULT_FILE_NAME "Archivo"
#define DEFAULT_FILE_PATH "/" DEFAULT_FILE_NAME

//Se llama cada vez que se tiene que mostrar un archivo en un directorio, para conocer sus atributos.
static int prueba_getattr(const char *path, struct stat *stbuf) {
	/* Esta implementacion le da el tipo archivo simple a cualquier archivo que se encuentre en
	 * el directorio raíz del punto de montaje y se llame "Archivo". A cualquier otra cosa, le da un
	 * tipo directorio.*/
	int res = 0;
	memset(stbuf, 0, sizeof(struct stat));
	//Si path es igual a "/" nos estan pidiendo los atributos del punto de montaje
	if (strcmp(path, "/") == 0) {
		/* stat contendrá todas las características asociadas al archivo recibido, entre otras:
		 * mode: tipo del archivo. IFDIR es para directorio e IFREG es para archivo simple.
		 * nlink: número de hardlinks (ver descripción más abajo).
		 * Ni idea para qué sirve | número. Sé que | es un OR binario, pero no cuál es la utilidad en estos casos.*/
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if (strcmp(path, DEFAULT_FILE_PATH) == 0) {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(DEFAULT_FILE_CONTENT); //Tamaño del archivo en bytes.
	} else {
		stbuf->st_mode = S_IFDIR;
		stbuf->st_nlink = 2;
	}

	if (strcmp(path, "") == 0)
		res = -ENOENT; //Error que indica archivo no encontrado.
	return res;
}

//Se invoca cada vez que se quieran conocer los archivos dentro de un directorio.
static int prueba_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
	/* Esta implementación impone 2 archivos por directorio, uno llamado Archivo y otro Subcarpeta. El tipo
	 * de cada uno se determina llamando a getattr (1 vez para cada uno).*/
	(void) offset;
	(void) fi;
	//if (strcmp(path, "/") != 0)
		//return -ENOENT;
	/* "." y ".." son entradas válidas, la primera es una referencia al directorio donde estamos parados
	* y la segunda indica el directorio padre. Éstos son hardlinks. Según tengo entendido, un hardlink es un
	* atributo obligatorio en un archivo. Si se fijan más arriba, un directorio tiene 2, "." y "..", y un
	* archivo simple tiene 1, que aparentemente es ".".
	* Después está el filler. No sé como carajo funciona, pero parece que completa buf con la cadena
	* segundo parámetro. El resto, creo que van por defecto en esos valores. Habría que investigarlo más.*/
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	filler(buf, DEFAULT_FILE_NAME, NULL, 0);
	filler(buf, "Subcarpeta", NULL, 0);
	return 0;
}

//Éstas son las operaciones del filesystem que FUSE va a "wrappear". Para el tp hay que agregar más.
static struct fuse_operations prueba_oper = {
		.getattr = prueba_getattr,
		.readdir = prueba_readdir,
};

int main(int argc, char *argv[])
{

	//El núcleo de FUSE. En teoría hay que invocarlo y olvidarse.
	return fuse_main(argc, argv, &prueba_oper, NULL);

	return 0;
}
