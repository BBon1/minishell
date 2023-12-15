#include "parser.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

//  VARIABLES GLOBALES
pid_t * hijos; // Array dinamico de pids
tub * tube; // Array dinámico de los pipes
tline * line; // Array dinamico de tlines
int m; // Variable para indicar que mandato toca ejecutar



void manejador_mandatos ();
void primer_plano();


int main() {
	// Poner todas las declaraciones de variables locales al principio del main (o funciones)
	char buf[1024];
	pid_t pid;	
	int i, p; // Variable local para los bucles for
	hijos = (pid_t*)calloc (5, sizeof(pid_t));  // inicializar lista dinamica
	tube = (tub *)calloc(4, sizeof(tub)); // inicializar lista dinamica
	trabajo * jobs_array = (trabajo*)calloc (10, sizeof(trabajo));  // inicializar lista dinamica
	trabajo auxJ; // Variable auxiliar para los jobs
	char s [200]; // Variable auxiliar
	int j = 1; // Contador de jobs
	char * token;
	int pid_valido = 0;
	int auxT;
	
	
	signal(SIGUSR1, manejador_mandatos); // Minishell llama al manejador para ejecutar mandatos
	signal(SIGUSR2, primer_plano);
	signal(2 ,SIG_IGN);  // Mini shell ignora Ctrl+C
	
	
	fprintf(stdout,"miniShell ==> Empezando minishell \nminiShell ==> ");	
	
	while (fgets(buf, 1024, stdin)) {  // MINISHELL
	
		// Comprobaciones del contenido del buffer
		if (strcmp("exit\n", buf) == 0){ // Escribir exit para poder salir de la minishell
			fprintf(stdout,"Saliendo de la minishell...\n");
			
			// Matar a los procesos que aún no han terminado
			for (i = 1; i < j; i++ ){
				if (waitpid(jobs_array[i].pid, NULL , WNOHANG) == 0){
					kill(9, jobs_array[i].pid);
				}
			}
			// Liberar memoria dinamica
			free(tube);
			free(hijos); 
			free(jobs_array);
			
			return 0;
			
			
		} else if (strcmp("\n", buf) == 0){ // No escribir nada 
			fprintf(stdout,"miniShell ==> ");
			continue;
			
			
		} else if (strcmp("jobs\n", buf) == 0){
			
			for (i = 1; i < j; i++ ){
				 
				if (jobs_array[i].pid == waitpid(jobs_array[i].pid, NULL , WNOHANG)){  
				// Ha terminado por su cuenta
					if (WEXITSTATUS(jobs_array[i].status) == 0){
						strcpy(s, "Done");
						jobs_array[i].view = 1;
					}
				} else if (waitpid(jobs_array[i].pid, NULL , WNOHANG) == -1) { // Terminado con error
						strcpy(s, "Error"); 
				} else { // No ha terminado
					if (WIFSTOPPED(jobs_array[i].status)){
						strcpy(s, "Stopped");
					} else if (WIFSIGNALED(jobs_array[i].status)){
						strcpy(s, "Signaled");
					} else {
						strcpy(s, "Running");
					}
				}
				// Limpiar los mandatos en jobs que hayan terminado
				if (jobs_array[i].view){ // Si se ha visto una vez, eliminar
					for (p = i; p < j-1; p++){
						jobs_array[p] = jobs_array[p+1];
					}
					j --;
				}
				
				fprintf(stdout,"[%d] %s       %s", jobs_array[i].index , s, jobs_array[i].name);
			}
			
			fprintf(stdout,"miniShell ==> ");
			continue;
			
			
		} else if (strcmp("fg\n", buf) == 0){ // Solo fg
		
			if (j > 1){
				auxT = jobs_array[1].pid;
				// Primer mandato en bg pasa a fg
				for (i = 1; i < j-1; i++){
					jobs_array[i] = jobs_array[i+1];
				}
				j--;
				fprintf(stdout,"Pasando el proceso %d a primer plano\n", auxT);
				//Que pueda morir el mandato pasado a primer plano
				kill(auxT, SIGUSR2);
				waitpid(auxT , NULL , 0);
			} else {
				fprintf(stderr, "No hay ningún mandato en segundo plano\n");
			}
			fprintf(stdout,"miniShell ==> ");
			continue;
			
		} else if (strstr(buf, "fg") != NULL){
			token = strtok(buf, " ");  // Quitar el fg
			token = strtok(NULL, " "); // PID
			auxT = atoi(token);
			
			// Comprobrar si el pid esta en jobs
			for (i = 1; i < j; i++){
				if (auxT == jobs_array[i].pid){
					pid_valido = 1;
					// Si está, actualizar la lista dinámica de jobs
					for (p = i; p < j-1; p++){
						jobs_array[p] = jobs_array[p+1];
					}
					j --;
				}
			}
			if (pid_valido){
				fprintf(stdout,"Pasando el proceso %d a primer plano\n", auxT);
				//Que pueda morir el mandato pasado a primer plano
				kill(auxT, SIGUSR2);
				waitpid(auxT , NULL , 0);
			} else {
				fprintf(stderr, "El PID pasado no es válido\n");
			}
			pid_valido = 0;
			fprintf(stdout,"miniShell ==> ");
			continue;
		}
		
		
		line = tokenize(buf);  // GET mandatos
		
		if (line==NULL) {
			fprintf(stderr,"Ha surgido un error al reservar espacio en memoria\n");
			continue;
		}
		
		
		// Redimensionar si fuera necesario
		if (line->ncommands > 5){
			hijos = realloc(hijos, line->ncommands * sizeof(pid_t));
			tube = realloc(tube, (line->ncommands-1) * sizeof(tub));
			if (hijos == NULL || tube == NULL){
				// Si surge algún problema al redimensionar, volver a asignar espacio de memoria
				hijos = (pid_t*)calloc (line->ncommands, sizeof(pid_t));  // Array dinamico de pids
				tube = (tub *)calloc(line->ncommands-1, sizeof(tub)); // Array dinámico de los pipes
			}
		}
		if (j > 10){
			jobs_array = realloc(jobs_array, (j*2) * sizeof(trabajo));
			if (jobs_array == NULL ){
				fprintf(stderr,"Ha surgido un error al reservar espacio en memoria, no caben más procesos en 2º plano\n");
			}
		}
		
		
		// Crear los pipes
		for (i = 0; i < line->ncommands-1; i++){ // crear los pipes antes de que crear los hijos
			pipe(tube[i].tuberia);
		}
		if ( !(line->background) ){ 
			signal(2 ,SIG_DFL); // Los hijos mueren con ctrl+c si se ejecutan en fg
		}
		//Crear un hijo para cada mandato, la variable global m cambia de valor al final del for	
		for (m = 0 ; m < line->ncommands; m ++){ 
			pid = fork();
			if (pid == 0){
				pause(); //Dejar esperandolos hasta recibir una señal
			} else {
				hijos[m] = pid;
			}
		}
		
		// Cerrar los pipes en la minishell
		for (i = 0; i < line->ncommands-1; i++){
			close (tube[i].tuberia[1]); 
			close (tube[i].tuberia[0]); 
		}
		
		// Despertar a todos los mandatos
		for (m = 0 ; m < line->ncommands; m ++){ 
			kill(hijos[m], SIGUSR1); 
		}
		
		// La minishell solo espera bloqueada si se ejecuta en primer plano, y solo espera al ultimo mandato
		if ( !(line->background) ){ 
			signal(2 ,SIG_IGN); // El padre no muere
			waitpid(hijos[line->ncommands-1], NULL, 0);
		} else {
			waitpid(hijos[line->ncommands-1], &auxJ.status , WNOHANG);
			auxJ.pid = hijos[line->ncommands-1]; 
			auxJ.index=j;
			auxJ.view = 0;
			strcpy(auxJ.name, buf);
			jobs_array[j] = auxJ;	
			fprintf(stdout, "[%d] %d\n", j, jobs_array[j].pid);	
			j++;
		}
		
		
			
		// Siguiente
		fprintf(stdout,"miniShell ==> ");	
	}
	return 0;
}

void manejador_mandatos (){
	int file_input, file_output, file_error;  //Ficheros utilizados si hay redirecciones de input, output o error
	int i ; // Variable local para los for
	
	if ( line->ncommands != 1 && m == line->ncommands-1 ){ // Ultimo hijo utimo mandato m, distinto de 0
	
		//  Redirección con pipes
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

		
		
		// Cerrar los pipes sobrantes después de haber despertado al resto de hijos
		for (i = 0; i < m-1; i++){
			close (tube[i].tuberia[1]); 
			close (tube[i].tuberia[0]); 
		}
		close (tube[m-1].tuberia[1]); 
		
		// Ejecutar mandato
		if (execvp(line->commands[m].filename , line->commands[m].argv) == -1){
			printf("%s: No se ha encontrado dicho comando\n", line->commands[m].filename);
			fprintf(stderr, "Error al ejecutar el programa\n");
			exit(1);
		}
		
		
		
	}else if ( line->ncommands != 1 && m > 0){ //Mandato intermedios Hijo - (m,0)
		
		//  Redirección con pipes
		dup2(tube[m-1].tuberia[0], 0); // pipe lectura -> stdin
		dup2(tube[m].tuberia[1], 1); //  pipe salida -> stdout

		// Cerrar los pipes sobrantes después de haber despertado al resto de hijos
		for (i = 0; i < m-1; i++){
			close (tube[i].tuberia[1]); 
			close (tube[i].tuberia[0]); 
		}
		for (i = m+1; i < line->ncommands-1; i++){
			close (tube[i].tuberia[1]); 
			close (tube[i].tuberia[0]); 
		}
		close (tube[m-1].tuberia[1]);	
		close (tube[m].tuberia[0]);
		
		// Ejecutar mandato
		if (execvp(line->commands[m].filename , line->commands[m].argv) == -1){
			printf("%s: No se ha encontrado dicho comando\n", line->commands[m].filename);
			fprintf(stderr, "Error al ejecutar el programa\n");
			exit(1);
		}
		
		
	} else { // Primer hijo, primer mandato - Hijo 0 / Un solo hijo
		
		//  Redirección con pipes y cierre de los sobrantes	
		for (i = line->ncommands-1; i > 0; i--){
			close (tube[i].tuberia[1]); 
			close (tube[i].tuberia[0]); 
		}
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
		if (execvp(line->commands[m].filename , line->commands[m].argv) == -1){
			printf("%s: No se ha encontrado dicho comando\n", line->commands[m].filename);
			fprintf(stderr, "Error al ejecutar el programa\n");
			exit(1);
		}
	}	
		
}

void primer_plano(){
	signal(2 ,SIG_DFL);
	return;
}
