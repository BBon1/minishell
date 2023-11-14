//      gcc -Wall -Wextra PruebaApartado1.c libparser_64.a -o PruebaApartado1 -static
// Created by Brenda Ferrante on 14/11/23.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "parser.h"

int main (){
    // Creamos el bufer donde se almacenara la linea de entrada:
    char buf[1024];
    tline * line;

    // Imprimimos el prompt:
    printf("msh > ");
    while (fgets(buf, 1024, stdin)) {

        // Tokenizamos la entrada para saber que mandatos y redirecciónes tenemos:
        line = tokenize(buf);

        // Si el mandato es un exit terminamos la ejecución de la MiniShell:
        if (strcmp(line->commands[0].argv[0], "exit") == 0) {
            break;

            // Si no, primero comprobamos cuantos mandatos nos pasan:
        } else {
            // Si la linea no contiene nada directamente ejecutamos de nuevo el prompt:
            if (line == NULL) {
                continue;
            }

            // Si tiene solo un mandato y no tiene redirecciónes:
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
            printf("msh > ");
        }
    }
    return 0;
}

