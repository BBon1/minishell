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
tub * tube; // Array dinámico de los pipes
tline * line; // Array dinamico de tlines
int m; // Variable para indicar que mandato toca ejecutar

void manejador_mandatos ();

int main() {
	// Poner todas las declaraciones de variables locales al principio del main (o funciones)
	char buf[1024];
	char check_exit [] = "exit\n";
	pid_t pid;	
	int i;
	
	signal(SIGUSR1, manejador_mandatos); // Minishell llama al manejador para ejecutar mandatos
	signal(2 ,SIG_IGN);  // Mini shell ignora Ctrl+C
	
	printf("miniShell ==> ");	
	while (fgets(buf, 1024, stdin)) {  // MINISHELL
		
		if (strcmp(check_exit, buf) == 0){ // Escribir exit para poder salir de la minishell
			printf("Saliendo de la minishell...\n");
			// Liberar memoria dinamica, cerrar pipes...
			free(tube);
			free(hijos); 
			// CLOSE PIPES
			return 0;
			
		} else if (strcmp("\n", buf) == 0){ // Escribir exit para poder salir de la minishell
			printf("minishell ==> ");
			continue;
		}
		
		line = tokenize(buf);  // GET mandatos
		
		if (line==NULL) {
			printf("Ha surgido un error al reservar espacio en memoria\n");
			continue;
		}
		
		// Crear los hijos necesarios
		hijos = (pid_t*)calloc (line->ncommands, sizeof(pid_t));
		
		tube = (tub *)calloc(line->ncommands-1, sizeof(tub));
		for (i = 0; i < line->ncommands; i++){
			pipe(tube[i].tuberia);
		}
		
		for (m = 0 ; m < line->ncommands; m ++){ //Crear un hijo para cada mandato, la variable global m cambia de valor al final del for	
			
			pid = fork();
			if (pid == 0){
				pause(); //Dejar esperandolos hasta recibir una señal
			} else {
				hijos[m] = pid;
				close(tube[m].tuberia[0]);
				close(tube[m].tuberia[1]);
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
	int i ;
	
	signal(2 ,SIG_DFL); // Los hijos si mueren con ctrl+c
	
	if ( line->ncommands != 1 && m == line->ncommands-1 ){ // Ultimo hijo utimo mandato m, distinto de 0
	
		//  REDIRECCION DE PIPES / entrada
		
		close (tube[m-1].tuberia[1]); 
		dup2(tube[m-1].tuberia[0], 0); // pipe  -> entrada 
		
		
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
	
	
		kill(hijos[m-1], SIGUSR1); // Despertar al resto de hijos
		
		
		// Ejecutar mandato
		execvp(line->commands[m].filename , line->commands[m].argv);
		fprintf(stderr, "Error al ejecutar el programa\n");
		exit(1);
		
	}else if (line->ncommands != 1 && m > 0){ //Mandato intermedios Hijo - (m,0)
		kill(hijos[m-1], SIGUSR1); // Despertar al resto de hijos
		
		//  REDIRECCION DE PIPES POR AQUI
		
		close (tube[m-1].tuberia[1]);	
		close (tube[m].tuberia[0]);
		dup2(tube[m-1].tuberia[0], 0); // pipe lectura -> stdin
		dup2(tube[m].tuberia[1], 1); //  pipe salida -> stdout
		
		
		// Ejecutar mandato
		execvp(line->commands[m].filename , line->commands[m].argv);
		fprintf(stderr, "Error al ejecutar el programa\n");
		exit(1);
		
	} else { // Primer hijo, primer mandato - Hijo 0 / Un solo hijo
		
		//  REDIRECCION DE PIPES
		close(tube[m].tuberia[0]); // cerrar el pipe[0]
		if (line->ncommands > 1){  // Si hay mas de 1 mandato, redireccionar salida
			dup2(tube[m].tuberia[1], 1); // pipe[0][1]  -> salida 
			
		} else {
			// Redirecciones de output y error
			if (line->redirect_output != NULL) {
				file_output = open(line->redirect_output , O_WRONLY | O_CREAT, 0666); 
				// Permisos resultantes -> 664, porque está umask 002
				// No permite escritura del archivo a todo el mundo, se puede cambiar el umask
				dup2(file_output ,1);
			}
			if (line->redirect_error != NULL) {
				file_error = open(line->redirect_error , O_WRONLY | O_CREAT, 0666);
				dup2(file_error, 2);
			}
		}
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
