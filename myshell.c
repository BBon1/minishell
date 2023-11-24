#include "parser.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

//  VARIABLES GLOBALES
pid_t * hijos;  // Array dinamico de pids
int * tub; // Array dinámico de los pipes
tline * line; // Array dinamico de tlines
int m; // Variable para indicar que mandato toca ejecutar

void manejador_mandatos ();

int main() {
	// Poner todas las declaraciones de variables locales al principio del main (o funciones)
	char buf[1024];
	int i; // Variable para los for
	char check_exit [] = "exit\n";
	pid_t pid;	
	
	signal(SIGUSR1, manejador_mandatos); // Minishell llama al manejador para ejecutar mandatos
	signal(2 ,SIG_IGN);  // Mini shell ignora Ctrl+C
	
	printf("miniShell ==> ");	
	while (fgets(buf, 1024, stdin)) {  // MINISHELL
		
		if (strcmp(check_exit, buf) == 0){ // Escribir exit para poder salir de la minishell
			printf("Saliendo de la minishell...\n");
			// Liberar memoria dinamica, cerrar pipes...
			free(line);
			free(hijos); 
			return 0;
			
		} else if (strcmp("\n", buf) == 0){ // Escribir exit para poder salir de la minishell
			printf("==> ");
			continue;
		}
		
		line = tokenize(buf);  // GET mandatos
		
		if (line==NULL) {
			printf("Ha surgido un error al reservar espacio en memoria\n");
			continue;
		}
		
		// Crear los hijos necesarios
		hijos = (pid_t*)calloc(line->ncommands, sizeof(pid_t));
		for (m = 0 ; m < line->ncommands; m ++){ //Crear un hijo para cada mandato, la variable global m cambia de valor al final del for
			pid = fork();
			if (pid == 0){
				pause(); //Dejar esperandolos hasta recibir una señal
			} else {
				hijos[m] = pid;
			}
		}
		
		// Despertar el primer hijo/mandato (ultimo)
		kill(hijos[line->ncommands-1], SIGUSR1);
		
		// La minishell solo espera bloqueada si se ejecuta en primer plano, y solo espera al ultimo mandato
		if ( !(line->background) ){ 
			waitpid(hijos[line->ncommands-1], NULL, 0);
		} else {
			waitpid(hijos[line->ncommands-1], NULL, WNOHANG);
		}
				
		// Siguiente
		printf("miniShell ==> ");	
	}

	return 0;
}




void manejador_mandatos (){
	int file_input, file_output, file_error;  //Ficheros utilizados si hay redirecciones de input, output o error
	
	signal(2 ,SIG_DFL); // Los hijos si mueren con ctrl+c
	
	if (m == line->ncommands-1){ // Ultimo hijo utimo mandato m
		kill(hijos[m-1], SIGUSR1); // Despertar al resto de hijos

		//  REDIRECCION DE PIPES POR AQUI s
		// dup2(pipe[m-1][1], 1);  pipe  -> salida std 
		sleep(m); // APAÑO
		
		// Redirecciones de output y error
		if (line->redirect_output != NULL) {
			file_output = open(line->redirect_output , O_WRONLY | O_CREAT, 0666); 
			// Permisos resultantes -> 664, porque está umask 002
			// No permite escritura del archivo a todo el mundo, se puede cambiar el umask
			dup2(file_output ,1);
		}
		if (line->redirect_error != NULL) {
			file_error = open(line->redirect_error , O_WRONLY | O_CREAT, 0666);
			dup2(file_error,2);
		}
		// Ejecutar mandato
		execvp(line->commands[m].filename , line->commands[m].argv);
		fprintf(stderr, "Error al ejecutar el programa\n");
		exit(1);
	}else if (m > 0){ //Mandato intermedios Hijo - (m,0)
		kill(hijos[m-1], SIGUSR1); // Despertar al resto de hijos
		
		//  REDIRECCION DE PIPES POR AQUI	
		// dup2(pipe[m+1][1], 0);  pipe  -> salida std 
		sleep(i); // APAÑO
		
		// Ejecutar mandato
		execvp(line->commands[m].filename , line->commands[m].argv);
		fprintf(stderr, "Error al ejecutar el programa\n");
		exit(1);
		
	} else { // Primer hijo, primer mandato - Hijo 0
	
		//Redirección input
		if (line->redirect_input != NULL){
			file_input = open(line->redirect_input , O_RDONLY);
			dup2(file_input ,0);
		}
		// Ejecutar mandato
		execvp(line->commands[m].filename , line->commands[m].argv);
		fprintf(stderr, "Error al ejecutar el programa\n");
		exit(1);
	}	
	
	 
		
}
