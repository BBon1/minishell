//      gcc -Wall -Wextra PruebaApartado2.c libparser_64.a -o PruebaApartado2 -static
// Created by Brenda Ferrante on 14/11/23.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "parser.h"

int main (){
    // Creamos el buffer donde se almacenara la línea de entrada:
    char buf[1024];
    tline * line;

    // Imprimimos el prompt:
    printf("msh > ");
    while (fgets(buf, 1024, stdin)) {

        // Tokenizamos la entrada para saber que mandatos y redirecciones tenemos:
        line = tokenize(buf);

        // Si el mandato es un exit terminamos la ejecución de la MiniShell:
        if (strcmp(line->commands[0].argv[0], "exit") == 0) {
            break;

            // Si no, primero comprobamos cuantos mandatos nos pasan:
        } else {
            // Si la línea no contiene nada directamente ejecutamos de nuevo el prompt:
            if (line == NULL) {
                continue;
            }

            /************************ Si tiene solo un mandato y no tiene redirecciones: ****************************/
            if (((line->ncommands == 1) && (line->redirect_input == NULL) )&& (line->redirect_output == NULL)){
                // Creamos directamente un hijo que ejecute el mandato pedido con los respectivos argumentos:
                pid_t  pid_1msr;
                pid_1msr = fork();


                if (pid_1msr < 0) {         // Error en la creación del hijo:
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

                /************************+++++++ Si tiene solo un mandato y tiene redirecciones: ****************************/
            else if ((line->ncommands == 1) && ((line->redirect_input != NULL) || (line->redirect_output != NULL))){

                // Creamos un hijo que ejecute el mandato pedido con los respectivos argumentos:
                pid_t  pid_1mcr;
                pid_1mcr = fork();

                if (pid_1mcr < 0) {         // Error en la creación del hijo:
                    fprintf(stderr, "Falló el fork() de un mandato con redirecciones.\n%s\n", strerror(errno));
                    return 1;

                } else if (pid_1mcr == 0) {      // Proceso Hijo
                    int fdO, fdI;
                    if (line->redirect_input != NULL){
                        // Redirección de entrada, tenemos que cambiar la entrada para que lea de un fichero que nos dan:
                        fdI = open(line->redirect_input, O_RDONLY);
                        if (fdI == -1){
                            fprintf(stderr, "El fichero: %s no existe.\n", line->redirect_input);
                        } else {
                            dup2(fdI,0);
                        }
                    }

                    if (line->redirect_output != NULL){
                        // Redirección de salida, tenemos que cambiar la salida para que escriba en un fichero que nos dan:
                        fdO = open(line->redirect_output, O_CREAT | O_APPEND);
                        dup2(fdO, 1);
                    }

                    execvp(line->commands[0].argv[0], line->commands[0].argv);
                    //Si llega aquí es que se ha producido un error en el execvp
                    fprintf(stderr, "Error al ejecutar el mandato: cod 2 %s\n", strerror(errno));
                    return 1;

                }
                else {      // Proceso Padre. (Tendrá que cerrar las redirecciones anteriores y volver a las originales)
                    wait (NULL);
                }
            }
            printf("msh > ");
        }
    }
    return 0;
}
