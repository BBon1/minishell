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

pid_t hijos[2] = {0, 0};
int i;
void manejador_hijo();
tline * line;
int pipe2Man[2];


int main (){
    // Creamos el buffer donde se almacenara la línea de entrada:
    char buf[1024];
    pipe(pipe2Man);


    // Imprimimos el prompt:
    printf("msh > ");
    while (fgets(buf, 1024, stdin)) {

        // Si la línea no contiene nada directamente ejecutamos de nuevo el prompt:
        if (buf[0] == '\n') {
            printf("msh > ");
            continue;
        }

        // Tokenizamos la entrada para saber que mandatos y redirecciones tenemos:
        line = tokenize(buf);

        // Si el mandato es un exit terminamos la ejecución de la MiniShell:
        if (strcmp(line->commands[0].argv[0], "exit") == 0) {
            break;

            // Si no, primero comprobamos cuantos mandatos nos pasan:
        } else {

            /********************************************************************************************************/
            /************************ Si tiene solo un mandato y no tiene redirecciones: ****************************/
            /********************************************************************************************************/

            if (((line->ncommands == 1) && (line->redirect_input == NULL) )&& (line->redirect_output == NULL)){

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
                signal(SIGUSR2, manejador_hijo);

                for(i = 0; i < com; i++) {
                    pid = fork();
                    if(pid == 0)
                    {
                        pause();
                    }
                    else
                    {
                        hijos[i] = pid;  //guardo el pid del hijo
                    }
                }


                //Bucle para esperar que terminen los hijos
                for(i = 0; i < com; i++)
                {
                    kill(hijos[i], SIGUSR2);
                    close(pipe2Man[0]);
                    close(pipe2Man[1]);
                    wait(NULL);
                }
//                printf("Todos los hijos terminaron\n");

            }
            printf("msh > ");
            continue;
        } // Termina el else de clasificar la entrada por n.º de argumentos

    }
    return 0;
}

void manejador_hijo()
{
    if (i == 0){
        // Redireccionamos para que escriba en el pipe y no en pantalla:
        dup2(pipe2Man[1], 1);
        close(pipe2Man[0]);
    } else if (i==1){
        // Redireccionamos para que lea del pipe y no de pantalla:
        dup2(pipe2Man[0], 0);
        close(pipe2Man[1]);
    }

    execvp(line->commands[i].argv[0], line->commands[i].argv);
    //Si llega aquí es que se ha producido un error en el execvp
    printf("Error al ejecutar el mandato: cod 2 %s\n", strerror(errno));
    exit(1);
}