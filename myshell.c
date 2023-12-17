#include "parser.h"
#include "job.h"

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
jobList * fgList;  // Array dinamico para procesos en fg
jobList * bgList;  // Array dinamico de procesos en bg
int mandato; // Variable para indicar que mandato toca ejecutar
/////////////////////////////////////////////////////////////////////////////////////////

//// DECLARACIÓN DE FUNCIONES ////////////////////////////////////////////////////////////
void manejador_hijos ();
void manejador_C();
void cd();
//////////////////////////////////////////////////////////////////////////////////////////

//// MAIN ////////////////////////////////////////////////////////////////////////////////
int main (){
    // VARIABLES  ////////////////////////////////////////////////////////////
    char buf[1024];
    pid_t pid;
    int i; // Variable local para los bucles for
    tJob auxJ; // Variable auxiliar para los jobs
    // Averiguamos el directorio en el que estamos trabajando
    char cwd[1024];
    const char * directorio = getcwd(cwd, sizeof(cwd));
    // Inicializar listas dinámicas
    hijos = (pid_t*)calloc (5, sizeof(pid_t));
    pipes = (tPipe *)calloc(4, sizeof(tPipe));
    initJobs(fgList);
    initJobs(bgList);

    // SIGNAL
    signal(2 , manejador_C); // Myshell ignora el ctr+c
    signal(SIGUSR1, manejador_hijos);

    // COMIENZA LA MYSHELL ///////////////////////////////////////////////////
    fprintf(stdout, "msh (%s)> ", directorio);
    while (fgets(buf, 1024, stdin)) {

        // Si la línea no contiene nada directamente ejecutamos de nuevo el prompt:
        if (strcmp("\n", buf) == 0) {
            fprintf(stdout,"msh %s)> ", directorio);
            continue;
        }

        line = tokenize(buf);

        if (line==NULL) {
            fprintf(stderr,"Ha surgido un error al reservar espacio en memoria\n");
            continue;
        } else if (strcmp("exit\n", line->commands[0].argv[0]) == 0){
            fprintf(stdout,"Saliendo de msh...\n");
            // Matar a los procesos que aún no han terminado
            // kill(9, ldJobs[i].pid);

            // Liberar memoria dinamica
            free(pipes);
            free(hijos);
            freeJobs(fgList);
            freeJobs(bgList);
            return 0;


        } else if (strcmp(line->commands[0].argv[0], "cd") == 0) {  // Comprobamos si es el mandato cd:
            cd();

        } else if ( strcmp("jobs\n", line->commands[0].argv[0]) == 0){ // jobs
                printJobs(bgList);
                fprintf(stdout,"miniShell ==> ");
            continue;

        } else if (strcmp("fg\n", line->commands[0].argv[0]) == 0){ // fg
            if (line->commands[0].argv[1] == NULL){  // Solo se ha puesto fg


            } else { // Se ha dado un pid
                if (isIn(line->commands[0].argv[1], bgList)){ // Comprobar pid
                    fprintf(stdout,"Pasando el proceso %d a primer plano\n", line->commands[0].argv[1]);
                    addJob();
                    deleteJob();
                    waitpid(line->commands[0].argv[1] , NULL , 0);
                } else {
                    fprintf(stderr, "El PID pasado no es válido\n");
                }
            }
            if (j > 1){
                    auxT = ldJobs[1].pid;
                    // Primer mandato en bg pasa a fg
                    for (i = 1; i < j-1; i++){
                        ldJobs[i] = ldJobs[i+1];
                    }
                    j--;
                    fprintf(stdout,"Pasando el proceso %d a primer plano\n", auxT);
                    //Que pueda morir el mandato pasado a primer plano
                    kill(auxT, SIGUSR2);
                    waitpid(auxT , NULL , 0);
            } else {
                    fprintf(stderr, "No hay ningún mandato en segundo plano\n");
            }
            fprintf(stdout,"msh ==> ");
            continue;


            }

        }

        // Redimensionar si fuera necesario  /////////////////////////////////////////////////////////
        if (line->ncommands > 5){
            hijos = realloc(hijos, line->ncommands * sizeof(pid_t));
            pipes = realloc(pipes, (line->ncommands-1) * sizeof(tPipe));
            if (hijos == NULL || pipes == NULL){
                // Si surge algún problema al redimensionar, volver a asignar espacio de memoria
                hijos = (pid_t*)calloc (line->ncommands, sizeof(pid_t));  // Array dinamico de pids
                pipes = (tPipe *)calloc(line->ncommands-1, sizeof(tPipe)); // Array dinámico de los pipes
            }
        }
        if (isFull(fgList)){
            reallocJobs(fgList);
        } else if (isFull(bgList)){
            reallocJobs(bgList);
        }

        // Crear los pipes  /////////////////////////////////////////////////////////////////////////////
        for (i = 0; i < line->ncommands-1; i++){ // crear los pipes antes de que crear los hijos
            pipe(pipes[i].tuberia);
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
            waitpid(hijos[line->ncommands-1], NULL, 0);
        } else {
            waitpid(hijos[line->ncommands-1], &auxJ.status , WNOHANG);
            auxJ.pid = hijos[line->ncommands-1];
            auxJ.index=j;
            auxJ.view = 0;
            strcpy(auxJ.name, buf);
            ldJobs[j] = auxJ;
            fprintf(stdout, "[%d] %d\n", j, ldJobs[j].pid);
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
    int i ; // Variable local para los bucles for

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
            fprintf(stderr,"%s: No se ha encontrado dicho comando\n", line->commands[mandato].filename);
            fprintf(stderr, "Error al ejecutar el programa\n");
            exit(1);
        }

    } else if ( line->ncommands != 1 && mandato > 0){ //Mandato intermedios Hijo ////////////////////////

        //  Redirección con pipes
        dup2(pipes[mandato-1].tuberia[0], 0); // pipe lectura -> stdin
        dup2(pipes[mandato].tuberia[1], 1); //  pipe salida -> stdout

        // Cerrar los pipes sobrantes después de haber despertado al resto de hijos
        for (i = 0; i < mandato-1; i++){
            close (pipes[i].tuberia[1]);
            close (pipes[i].tuberia[0]);
        }
        for (i = mandato+1; i < line->ncommands-1; i++){
            close (pipes[i].tuberia[1]);
            close (pipes[i].tuberia[0]);
        }
        close (pipes[mandato-1].tuberia[1]);
        close (pipes[mandato].tuberia[0]);

        // Ejecutar mandato
        if (execvp(line->commands[mandato].filename , line->commands[mandato].argv) == -1){
            fprintf(stderr,"%s: No se ha encontrado dicho comando\n", line->commands[mandato].filename);
            fprintf(stderr, "Error al ejecutar el programa\n");
            exit(1);
        }


    } else { // Primer hijo, primer mandato - Hijo 0 / Un solo hijo ///////////////////////////////////

        //  Redirección con pipes y cierre de los sobrantes
        for (i = line->ncommands-1; i > 0; i--){
            close (pipes[i].tuberia[1]);
            close (pipes[i].tuberia[0]);
        }
        close(pipes[mandato].tuberia[0]); // cerrar el pipe[0]
        if (line->ncommands > 1){  // Si hay mas de 1 mandato, redireccionar salida
            dup2(pipes[mandato].tuberia[1], 1); // pipe[0][1]  -> salida
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
        if (execvp(line->commands[mandato].filename , line->commands[mandato].argv) == -1){
            fprintf(stderr,"%s: No se ha encontrado dicho comando\n", line->commands[mandato].filename);
            fprintf(stderr, "Error al ejecutar el programa\n");
            exit(1);
        }
    }

}

void manejador_C(){

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

