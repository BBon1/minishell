#include "parser.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>


int
main(void) {
	char buf[1024];
	char check_exit [] = "exit\n";
	tline * line;
	int i,j;

	printf("shell==> ");	
	while (fgets(buf, 1024, stdin)) {
		
		if (strcmp(check_exit, buf) == 0){ // Escribir exit para poder salir de la minishell
			printf("Saliendo de la minishell...\n");
			return 0;
		}
		
		line = tokenize(buf);
		if (line==NULL) {
			printf("line es NULL\n");
			continue;
		}
		
		
		
		pid_t pid = fork();
		
		if (pid == 0){ //MANDATO
			if (line->redirect_input != NULL){
				int file_input = open(line->redirect_input , O_RDONLY);
				dup2(file_input ,0);
			}
			if (line->redirect_output != NULL) {
				int file_output = open(line->redirect_output , O_WRONLY | O_CREAT, 0666); 
				// Permisos resultantes -> 664, porque está umask 002
				// No permite escritura del archivo a todo el mundo, se puede cambiar el umask
				dup2(file_output ,1);
			}
			if (line->redirect_error != NULL) {
				int file_error = open(line->redirect_error , O_WRONLY | O_CREAT, 0666);
				dup2(file_error,2);
			}
			
			execvp(line->commands->filename , line->commands->argv);
			
			fprintf(stderr, "Error al ejecutar el programa\n");
			exit(1);
			
		} else { // MINISHELL
			for (i=0; i<line->ncommands; i++) {
				printf("orden %d (%s):\n", i, line->commands[i].filename);
				for (j=0; j<line->commands[i].argc; j++) {
					printf("  argumento %d: %s\n", j, line->commands[i].argv[j]);
				}
			}
			if ( !(line->background) ){ 
				wait(NULL); //Solo se espera si se ejecuta en primer plano
			}
		}
		
		
		
		
		printf("shell==> ");	
	}
	return 0;
}
