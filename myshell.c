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
tline * line; // Array dinamico de tlines
int * status;
static int contador = -1; // Variable para indicar que mandato toca ejecutar
int i;

void manejador_mandatos ();

int main() {
	// Poner todas las declaraciones de variables locales al principio del main (o funciones)
	char buf[1024];
	char check_exit [] = "exit\n";
	
	pid_t pid;
	hijos = (pid_t*)calloc(10, sizeof(pid_t));
	status = (int*)calloc(10, sizeof(int));
	int file_input, file_output, file_error;
	
	signal(SIGUSR1, manejador_mandatos); // Minishell llama al manejador para ejecutar mandatos
	signal(2 ,SIG_IGN);  // Mini shell ignora Ctrl+C
	
	
	printf("==> ");	
	while (fgets(buf, 1024, stdin)) {  // MINISHELL
		
		if (strcmp(check_exit, buf) == 0){ // Escribir exit para poder salir de la minishell
			printf("Saliendo de la minishell...\n");
			free(hijos);
			free(status);
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
		
		//Redirección input
		if (line->redirect_input != NULL){
			file_input = open(line->redirect_input , O_RDONLY);
			dup2(file_input ,0);
		}
		
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
		
		
		// Crear los hijos necesarios
		signal(2 ,SIG_DFL); // Los hijos si mueren con ctrl+c
		for (i = 0 ; i < line->ncommands; i ++){ //Crear los hijos de cada mandato
			
			pid = fork();
			if (pid == 0){
				pause(); //Dejar esperandolos hasta recibir una señal
			} else {
				hijos[i] = pid;
			}
		}
		signal(2 ,SIG_IGN); // La minishell vuelve a ignorar
		
		
		// Ejecutar el primer mandato
		
		kill(hijos[line->ncommands-1], SIGUSR1);
		waitpid(hijos[line->ncommands-1], &status[0], 0);
		
		
		
		
		// La minishell solo espera si se ejecuta en primer plano, y solo espera al ultimo mandato
		//if ( !(line->background) ){ 
		//	waitpid(hijos[0], NULL, 0);
		//}
				
		
		
		//Ya se han terminado todos los mandatos
		printf("==> ");	
	}

	return 0;
}




void manejador_mandatos (){
	contador = i;
	// contador = line->ncommands - (i+1);
	
	if (i == line->ncommands-1){ // Ultimo hijo utimo mandato n
		kill(hijos[i-1], SIGUSR1); // Despertar al resto de hijos
		sleep(i);
		
		execvp(line->commands[contador].filename , line->commands[contador].argv);
		fprintf(stderr, "Error al ejecutar el programa\n");
		exit(1);
	}
	
	if (i > 0){ //Mandato intermedios Hijo - (n,0)
		kill(hijos[i-1], SIGUSR1); // Despertar al resto de hijos
		sleep(i);
		
		execvp(line->commands[contador].filename , line->commands[contador].argv);
		fprintf(stderr, "Error al ejecutar el programa\n");
		exit(1);
		
	} else { // Primer hijo, primer mandato - Hijo 0
		execvp(line->commands[contador].filename , line->commands[contador].argv);
		fprintf(stderr, "Error al ejecutar el programa\n");
		exit(1);
	}	 
		
}
