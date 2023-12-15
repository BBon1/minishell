//      gcc -Wall -Wextra VariosMandatos.c libparser_64.a -o VariosMand -static
// Created by Brenda Ferrante on 14/11/23.
//

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>

#include "parser.h"

// Estructura para las tuberías:
typedef struct {
    int p[2];
} tPipe;

// Variables globales:
// Pids:
pid_t * hijos;  // Para el array dinámico de pids de hijos

int mandato;    // Indica el mandato en ejecución
tline * line;   // Entrada

// Pipes
tPipe * pipes; // Para el array dinámico de pipes

// Funciones auxiliares
void manejador_hijosNMandatos();
void sigint_handler();
void cd();


int main (){

    // Establecer el manejador de señales para SIGINT (ignoramos el ctr+c)
    signal(SIGINT, sigint_handler);

    // Creamos el buffer donde se almacenara la línea de entrada:
    char buf[1024];
    pid_t pid;

    // Averiguamos el directorio en el que estamos trabajando
    char cwd[1024];
    const char * directorio = getcwd(cwd, sizeof(cwd));


    // Imprimimos el prompt:
    printf("msh (%s)> ", directorio);
    while (fgets(buf, 1024, stdin)) {

        // Comprobamos si hay hijos sin terminar en el background
//         comprobarBG();


        // Si la línea no contiene nada directamente ejecutamos de nuevo el prompt:
        if (buf[0] == '\n') {
            printf("msh %s)> ", directorio);
            continue;
        }


        // Tokenizamos la entrada para saber que mandatos y redirecciones tenemos:
        line = tokenize(buf);

        // Si el mandato es un exit terminamos la ejecución de la MiniShell: (primero debemos matar todos los hijos que sigan en el bg)
        if (strcmp(line->commands[0].argv[0], "exit") == 0) {
// Matar hijos BG
            break;

            // Si no, primero comprobamos cuantos mandatos nos pasan:
        } else {

            /************************************************************************************************************/
            /******************************* Si tiene solo un mandato y tiene redirecciones: ****************************/
            /************************************************************************************************************/

            if (line->ncommands == 1){

                // Comprobamos si es el mandato cd:
                if (strcmp(line->commands[0].argv[0], "cd") == 0) {
                    cd();
                } else {
                    // Creamos un hijo que ejecute el mandato pedido con los respectivos argumentos:
                    pid_t  pid_1mcr;
                    pid_1mcr = fork();

                    if (pid_1mcr < 0) {         // Error en la creación del hijo:
                        fprintf(stderr, "Falló el fork() de un mandato con redirecciones.\n%s\n", strerror(errno));
                        return 1;

                    } else if (pid_1mcr == 0) {      // Proceso Hijo

                        if (line->redirect_input != NULL){
                            // Redirección de entrada, tenemos que cambiar la entrada para que lea de un fichero que nos dan:
                            int fdI = open(line->redirect_input, O_RDONLY);
                            if (fdI == -1){
                                printf("El fichero no existe.");
                            } else {
                                dup2(fdI,0);
                            }
                        }

                        if (line->redirect_output != NULL){
                            // Redirección de salida, tenemos que cambiar la salida para que escriba en un fichero que nos dan:
                            int fdO = open(line->redirect_output, O_CREAT | O_WRONLY);
                            dup2(fdO, 1);
                        }

                        execvp(line->commands[0].argv[0], line->commands[0].argv);
                        //Si llega aquí es que se ha producido un error en el execvp
                        printf("Error al ejecutar el mandato: cod 2 %s\n", strerror(errno));
                        return 1;

                    }
                    else {      // Proceso Padre.
                        if ( (line->background) == 0 ){
                            wait(NULL);
                        } else {
                            waitpid(pid_1mcr, NULL, WNOHANG);
                        }
                    }
                }
            }

                /*****************************************************************************************************************/
                /******************************* Si tienen más de un mandato y redirecciones: **********************************/
                /*****************************************************************************************************************/

            else if(line->ncommands > 1) {
                int numeroComandos = line->ncommands;
                signal(SIGUSR2, manejador_hijosNMandatos);

                // Creamos los arrays dinámicos de pids y pipes:
                pipes = (tPipe *)calloc(numeroComandos-1, sizeof(tPipe));
                hijos = (int *)calloc(numeroComandos, sizeof(int));


                // Tenemos que unir todos los mandatos que se ejecutarán, cada uno en un hijo, mediante pipes:
                // Creamos los pipes necesarios:
                for (mandato = 0; mandato < numeroComandos -1; mandato++){
                    pipe(pipes[mandato].p);
                }

                // Creamos los hijos necesarios (uno por comando):
                for (mandato = 0 ; mandato < numeroComandos; mandato ++){
                    pid = fork();
                    if (pid == 0){
                        pause(); // Se quedan esperando la señal
                    } else {
                        hijos[mandato] = pid;
                    }
                }

                // Empezamos a ejecutar los hijos con kill:
                kill(hijos[numeroComandos-1], SIGUSR2);

                // Cerramos todos los pipes de la minishell porque no los usamos:
                for (int a = 0; a < line->ncommands - 1; a++){
                    close (pipes[a].p[1]);
                    close (pipes[a].p[0]);
                }

                // Esperamos a los hijos:
                if ( (line->background) == 0 ){
                    for (int i = 0; i < numeroComandos-1; i++) {
                        wait(NULL);
                    }
                } else {
                    waitpid(hijos[line->ncommands-1], NULL, WNOHANG);
                }

            }

            directorio = getcwd(cwd, sizeof(cwd));
            printf("msh (%s)> ", directorio);
            continue;
        } // Termina el else de clasificar la entrada por n.º de argumentos

    }

    // Liberamos la memoria de los arrays dinámicos usados previamente:
    free(hijos);
    free(pipes);
    free(backG);
    return 0;
}



////////////////////// FUNCIONES AUXILIARES //////////////////////////

// Manejador para más mandatos:
void manejador_hijosNMandatos(){


    if (mandato == line->ncommands-1){      // Es el último mandato que hay que ejecutar:
        // Al ser el último mandato para su entrada leerá de la salida del último pipe:
        dup2(pipes[mandato-1].p[0], 0);

        // Si tiene la línea redirección de salida:
        if (line->redirect_output != NULL){
            // Redirección de salida, tenemos que cambiar la salida para que escriba en un fichero que nos dan:
            int salida = open(line->redirect_output, O_CREAT | O_WRONLY);
            dup2(salida, 1);
        }

        // Mandaremos la señal al siguiente hijo:
        kill(hijos[mandato-1], SIGUSR2);

        // Cierra todos los pipes que no usa:
        for (int p = 0; p < mandato - 1; p++){
            close (pipes[p].p[1]);
            close (pipes[p].p[0]);
        }
        close (pipes[mandato-1].p[1]);

        // Finalmente, se ejecuta el comando (ya se han ejecutado los anteriores):
        execvp(line->commands[mandato].argv[0], line->commands[mandato].argv);
        //Si llega aquí es que se ha producido un error en el execvp
        printf("Error al ejecutar el mandato: cod 4.3 %s\n", strerror(errno));
        exit(1);

    } else if ((mandato < line->ncommands-1) && (mandato > 0)) {    // Son comandos intermedios (ni el primero ni el último):
        // Redirigimos entrada y salida:
        // Lee del pipe con número anterior a su número de mandato:
        dup2(pipes[mandato-1].p[0],0);
        // Escribe en el pipe con su mismo número de mandato:
        dup2(pipes[mandato].p[1],1);

        // Manda la señal al siguiente hijo:
        kill(hijos[mandato-1], SIGUSR2);

        // Cierra los pipes que no usa:
        for (int i = 0; i < mandato - 1; i++) {
            close(pipes[i].p[0]);
            close(pipes[i].p[1]);
        }
        for (int j = mandato+1; j < line->ncommands - 1; j++) {
            close(pipes[j].p[0]);
            close(pipes[j].p[1]);
        }
        close(pipes[mandato-1].p[1]);
        close(pipes[mandato].p[0]);

        // Ejecuta el comando:
        execvp(line->commands[mandato].argv[0], line->commands[mandato].argv);
        //Si llega aquí es que se ha producido un error en el execvp
        printf("Error al ejecutar el mandato: cod 4.2 %s\n", strerror(errno));
        exit(1);

    } else {    // Es el primer mandato:
        // Tendrá que escribir su salida en la entrada del primer pipe
        dup2(pipes[mandato].p[1],1);
        // Tiene que cerrar todo el resto de pipes que no utiliza:
        for (int c = 1; c < line->ncommands-1; c++) {
            close(pipes[c].p[0]);
            close(pipes[c].p[1]);
        }
        close(pipes[0].p[0]);
        // Si tiene redirección de entrada:
        if (line->redirect_input != NULL){
            // Redirección de entrada, tenemos que cambiar la entrada para que lea de un fichero que nos dan:
            int entrada = open(line->redirect_input, O_RDONLY);
            if (entrada == -1){
                printf("El fichero no existe.");
            } else {
                dup2(entrada, 0);
            }
        }

        // Ejecutamos el mandato:
        execvp(line->commands[mandato].argv[0], line->commands[mandato].argv);
        //Si llega aquí es que se ha producido un error en el execvp
        printf("Error al ejecutar el mandato: cod 4.1 %s\n", strerror(errno));
        exit(1);
    }
}


void sigint_handler() {
    // Manejador para ignorar la señal SIGINT (Ctrl+C) de la shell.
}


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