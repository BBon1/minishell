#include "parser.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>   // Libreria para los ficheros
#include <errno.h>   // Errores
#include <signal.h>

//// VARIABLES GLOBALES /////////////////////////////////////////////////////////////////
tline * line; // Array dinamico de tlines
tPipe * pipes; // Array dinámico de los pipes
pid_t * hijos; // Array dinamico de pids de los hijos
int mandato; // Variable para indicar que mandato toca ejecutar
/////////////////////////////////////////////////////////////////////////////////////////

//// DECLARACIÓN DE FUNCIONES ////////////////////////////////////////////////////////////
void manejador_hijos ();
void cd();
//////////////////////////////////////////////////////////////////////////////////////////

//// MAIN ////////////////////////////////////////////////////////////////////////////////
int main (){
  // VARIABLES  ////////////////////////////////////////////////////////////
  char buf[1024];
  pid_t pid;
  // Averiguamos el directorio en el que estamos trabajando
  char cwd[1024];
  const char * directorio = getcwd(cwd, sizeof(cwd));
  // Inicializar listas dinámicas
  hijos = (pid_t*)calloc (5, sizeof(pid_t));  
	pipes = (tPipe *)calloc(4, sizeof(tPipe)); 
	tJob * ldJobs = (tJob*)calloc (10, sizeof(tJob)); 
  // Auxiliares
  int i, p; // Variable local para los bucles for
  trabajo auxJ; // Variable auxiliar para los jobs
	char s [200]; // Variable auxiliar
	int j = 1; // Contador de jobs
	char * token;
	int pid_valido = 0;
	int auxT;
  
  // SIGNAL
  signal(SIGINT, sigint_handler);  // Myshell ignora el ctr+c
  signal(SIGUSR1, manejador_mandatos);

  // COMIENZA LA MYSHELL ///////////////////////////////////////////////////
  fprintf(stdout, "msh (%s)> ", directorio);
  while (fgets(buf, 1024, stdin)) {
    
        // Si la línea no contiene nada directamente ejecutamos de nuevo el prompt:
        if (strcmp("\n", buf) == 0) {
            fprintf(stdout,"msh %s)> ", directorio);
            continue;
        } else if (strcmp("exit\n", buf) == 0){
            fprintf(stdout,"Saliendo de myShell...\n");
            // Matar a los procesos que aún no han terminado
            for (i = 1; i < j; i++ ){  
                if (waitpid(jobs_array[i].pid, NULL , WNOHANG) == 0){
					          kill(9, jobs_array[i].pid);
				        }
            }
			        // Liberar memoria dinamica
			        free(pipes);
			        free(hijos); 
			        free(ldJobs);
			        return 0;
        }

      

    
        line = tokenize(buf);  
		
    		if (line==NULL) {
    			fprintf(stderr,"Ha surgido un error al reservar espacio en memoria\n");
    			continue;
    		} else if (strcmp(line->commands[0].argv[0], "cd") == 0) {  // Comprobamos si es el mandato cd:
          cd();
        } else if (){     ////////// JOBS y FG

      else if (strcmp("jobs\n", buf) == 0){
			
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
          
        }
    		
    		// Redimensionar si fuera necesario  /////////////////////////////////////////////////////////
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
    		} ///////////////////////////////////////////////////////////////////////////////////////////////
    		
    		// Crear los pipes  /////////////////////////////////////////////////////////////////////////////
    		for (i = 0; i < line->ncommands-1; i++){ // crear los pipes antes de que crear los hijos
    			pipe(tube[i].tuberia);
    		} 
    
    		if ( !(line->background) ){ 
    			signal(2 ,SIG_DFL); // Los hijos mueren con ctrl+c si se ejecutan en fg
    		}
    
    		//Crear un hijo para cada mandato	 /////////////////////////////////////////////////////////////
    		for (i = 0 ; i < line->ncommands; i ++){ 
    			pid = fork();
    			if (pid == 0){
    				pause(); //Dejar esperandolos hasta recibir una señal
    			} else {
    				hijos[i] = pid;
    			}
    		}
    		
    		// Cerrar los pipes en la myshell /////////////////////////////////////////////////////////////
    		for (i = 0; i < line->ncommands-1; i++){
    			close (pipes[i].tuberia[1]); 
    			close (pipes[i].tuberia[0]); 
    		} //////////////////////////////////////////////////////////////////////////////////////////////
    		
    		// Despertar a todos los mandatos  //////////////////////////////////////////////////////////////
    		for (mandato = 0 ; mandato < line->ncommands; mandato  ++){ 
    			kill(hijos[mandato], SIGUSR1); 
    		} ///////////////////////////////////////////////////////////////////////////////////////////////
    		
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

    directorio = getcwd(cwd, sizeof(cwd));
    fprintf(stdout,"msh (%s)> ", directorio);
  
  }
  return 0;
}


//// MANEJADOR ///////////////////////////////////////////////////////////////////////////
void manejador_hijos (){
	int file_input, file_output, file_error;  //Ficheros utilizados si hay redirecciones de input, output o error
	int i ; // Variable local para los for
	
	if ( line->ncommands != 1 && mandato == line->ncommands-1 ){ // Ultimo hijo utimo mandato m, distinto de 0 ////////////
		//  Redirección de entrada
		dup2(pipes[mandato-1].tuberia[0], 0);  
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
		for (i = 0; i < mandato-1; i++){
			close (pipes[i].tuberia[1]); 
			close (pipes[i].tuberia[0]); 
		}
		close (pipes[mandato-1].tuberia[1]); 
		
		// Ejecutar mandato
		if (execvp(line->commands[mandato].filename , line->commands[mandato].argv) == -1){
			printf("%s: No se ha encontrado dicho comando\n", line->commands[mandato].filename);
			fprintf(stderr, "Error al ejecutar el programa\n");
			exit(1);
		}
		
	}else if ( line->ncommands != 1 && m > 0){ //Mandato intermedios Hijo - (mandato,0) 
		
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
            // Redirección de entrada, tenemos que cambiar la entrada para que lea de un fichero que nos dan:
            file_input = open(line->redirect_input , O_RDONLY);
            if (file_input == -1){
                fprintf(stderr, "El fichero no existe.");
            } else {
                dup2(file_input ,0);
            }
      }

		
		// Ejecutar mandato
		if (execvp(line->commands[m].filename , line->commands[m].argv) == -1){
			printf("%s: No se ha encontrado dicho comando\n", line->commands[m].filename);
			fprintf(stderr, "Error al ejecutar el programa\n");
			exit(1);
		}
	}	
		
}

//// FUNCION CD /////////////////////////////////////////////////////////////////////////
void cd(){
    if (line->commands[0].argv[1] == NULL) {
        // Si no se proporciona un argumento, cambiar al directorio HOME
        const char *home_dir = getenv("HOME");
        if (home_dir != NULL) {
            chdir(home_dir);
        } else {
            perror("cd");
        }
    } else {
        // Cambiar al directorio especificado
        if (chdir(line->commands[0].argv[1]) != 0) {
            perror("cd");
        }
    }
}

