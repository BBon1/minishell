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

// Estructura para las tuberias:
typedef struct {
    int p[2];
} tPipe;

// Variables globales:
// Pids:
pid_t hijos2Mandatos[2] = {0, 0};
pid_t * hijos;  // Para el array dinamico de pids de hijos

int i;
tline * line;   // Entrada

// Pipes
int pipe2Mandatos[2];
tPipe * pipes; // Para el array dinamico de pipes

// Funciones auxiliares
void manejador_hijo2Mandatos();
void sigint_handler();


int main (){

    // Establecer el manejador de señales para SIGINT (ingnoramos el ctr+c)
    signal(SIGINT, sigint_handler);

    // Creamos el buffer donde se almacenara la línea de entrada:
    char buf[1024];
    pipe(pipe2Mandatos);

    // Averiguamos el directorio en el que estamos trabajando
    char cwd[1024];
    const char * directorio = getcwd(cwd, sizeof(cwd));


    // Imprimimos el prompt:
    printf("msh (%s)> ", directorio);
    while (fgets(buf, 1024, stdin)) {

        // Comprobamos si hay hijos sin terminar en el background
//        comprobarBG();


        // Si la línea no contiene nada directamente ejecutamos de nuevo el prompt:
        if (buf[0] == '\n') {
            printf("msh > ");
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

            /********************************************************************************************************/
            /************************ Si tiene solo un mandato y no tiene redirecciones: ****************************/
            /********************************************************************************************************/

            if (((line->ncommands == 1) && (line->redirect_input == NULL) )&& (line->redirect_output == NULL)){

                // Comprobamos si es el mandato cd:
                if (strcmp(line->commands[0].argv[0], "cd") == 0) {
                    if (line->commands[0].argv[1] == NULL) {
                        // Si no se proporciona un argumento, cambiar al directorio HOME
                        const char *home_dir = getenv("HOME");
                        if (home_dir != NULL) {
                            directorio = home_dir;
                            chdir(home_dir);
                        } else {
                            perror("cd");
                        }
                    } else {
                        // Cambiar al directorio especificado
                        if (chdir(line->commands[0].argv[1]) != 0) {
                            directorio = line->commands[0].argv[1];
                            perror("cd");
                        }
                    }
                } else {

                    // Creamos directamente un hijo que ejecute el mandato pedido con los respectivos argumentos:
                    pid_t  pid_1msr;
                    pid_1msr = fork();


                    if (pid_1msr < 0) {         // Error en la creacion del hijo:
                        fprintf(stderr, "Falló el fork() de un mandato sin redirecciones.\n%s\n", strerror(errno));
                        return 1;

                    } else if (pid_1msr == 0) {      // Proceso Hijo
                        execvp(line->commands[0].argv[0], line->commands[0].argv);
                        //Si llega aquí es que se ha producido un error en el execvp
                        printf("Error al ejecutar el comando: cod 1 %s\n", strerror(errno));
                        return 1;

                    }
                    else {      // Proceso Padre.
                        wait (NULL);
                    }
                }
            }


                /************************************************************************************************************/
                /******************************* Si tiene solo un mandato y tiene redirecciones: ****************************/
                /************************************************************************************************************/

            else if ((line->ncommands == 1) && ((line->redirect_input != NULL) || (line->redirect_output != NULL))){

                // Creamos un hijo que ejecute el mandato pedido con los respectivos argumentos:
                pid_t  pid_1mcr;
                pid_1mcr = fork();

                if (pid_1mcr < 0) {         // Error en la creacion del hijo:
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
                    wait (NULL);
                }
            }


                /*****************************************************************************************************************/
                /*******************************+++++++ Si tienen dos mandatos y redirecciones: **********************************/
                /*****************************************************************************************************************/

            else if (line->ncommands == 2) {
                int com = line->ncommands;
                pid_t pid;
                signal(SIGUSR2, manejador_hijo2Mandatos);

                for(i = 0; i < com; i++) {
                    pid = fork();
                    if(pid == 0)
                    {
                        pause();
                    }
                    else
                    {
                        hijos2Mandatos[i] = pid;  //guardo el pid del hijo
                    }
                }


                //Bucle para esperar que terminen los hijos2Mandatos
                for(i = 0; i < com; i++)
                {
                    kill(hijos2Mandatos[i], SIGUSR2);
                    close(pipe2Mandatos[0]);
                    close(pipe2Mandatos[1]);
                    wait(NULL);
                }
//                printf("Todos los hijos terminaron\n");

            }


                /*****************************************************************************************************************/
                /******************************* Si tienen más de dos mandatos y redirecciones: **********************************/
                /*****************************************************************************************************************/

            else if(line->ncommands > 2) {
                // Creamos los arrays dinamicos
                pipes = (tPipe *)calloc(line->ncommands-1,sizeof(tPipe));
                hijos = (int *)calloc(line->ncommands, sizeof(int));
            }

            directorio = getcwd(cwd, sizeof(cwd));
            printf("msh (%s)> ", directorio);
            continue;
        } // Termina el else de clasificar la entrada por n.º de argumentos

    }
    free(hijos);
    free(pipes);
    return 0;
}

void manejador_hijo2Mandatos()
{
    if (i == 0){
        // Redireccionamos para que escriba en el pipe y no en pantalla:
        dup2(pipe2Mandatos[1], 1);
        close(pipe2Mandatos[0]);
    } else if (i==1){
        // Redireccionamos para que lea del pipe y no de pantalla:
        dup2(pipe2Mandatos[0], 0);
        close(pipe2Mandatos[1]);
    }

    execvp(line->commands[i].argv[0], line->commands[i].argv);
    //Si llega aquí es que se ha producido un error en el execvp
    printf("Error al ejecutar el mandato: cod 2 %s\n", strerror(errno));
    exit(1);
}

void sigint_handler() {
    // Manejador para la señal SIGINT (Ctrl+C) de la shell
}