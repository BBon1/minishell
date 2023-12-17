#include "jobs.h"
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


void initJobs(jobList * nuevaLista){
    nuevaLista.contador = 1;
    nuevaLista.index = 0;
    nuevaLista.maximo = 10;
    nuevaLista.listaJobs = (tJob*)calloc (10, sizeof(tJob));
    nuevaLista->listaJobs->pids = (pid_t *)calloc (5, sizeof(pid_t));
}

void initJob(tJob * nuevo ,int in, pid_t last, pid_t * listaPids, int st, char * n){
    nuevo->index = in;
    nuevo->lastPid = last;
    nuevo->pids = listaPids;
    nuevo-> status = st;
    strcpy(nuevo->name, n);
    nuevo->delete = 0;
}

void initJobNoStatus(tJob * nuevo ,int in, pid_t last, pid_t * listaPids, char * n){
    nuevo->index = in;
    nuevo->lastPid = last;
    nuevo->pids = listaPids;
    nuevo->status = NULL;
    strcpy(nuevo->name, n);
    nuevo->delete = 0;
}

int isFull(jobList * lista){
    return lista.index == lista.maximo;
}


void reallocJobs(jobList * lista){
    lista.listaJobs = realloc(lista.listaJobs, (lista.maximo*2) * sizeof(tJob));
    if (lista.listaJobs == NULL ){
        fprintf(stderr,"Ha surgido un error al reservar espacio en memoria\n");
    }
    lista->maximo *= 2;
}

void printJobs(jobList * lista){
    int i ;
    char s [200]; // Variable auxiliar

    for (i = 0; i < lista.index; i++ ){

        if (lista.listaJobs[i].lastPid == waitpid(lista.listaJobs[i].lastPid, NULL , WNOHANG)){
            // Ha terminado por su cuenta
            if (WEXITSTATUS(lista.listaJobs[i].status) == 0){
                strcpy(s, "Done");
                lista.listaJobs[i].delete = 1;
            }
        } else if (waitpid(lista.listaJobs[i].lastPid, NULL , WNOHANG) == -1) { // Terminado con error
            strcpy(s, "Error");
        } else { // No ha terminado == 0
            if (WIFSTOPPED(lista.listaJobs[i].status)){
                strcpy(s, "Stopped");
            } else if (WIFSIGNALED(lista.listaJobs[i].status)){
                strcpy(s, "Signaled");
            } else {
                strcpy(s, "Running");
            }
        }

        fprintf(stdout,"[%d] %s       %s", lista.listaJobs[i].index , s, lista.listaJobs[i].name);
    }
    // Limpiar los mandatos que hayan terminado y ya hayamos visto una vez
    update(lista);
}

void addJob(tJob * newJob, jobList * lista){
    lista->listaJobs = newJob;
    lista->contador ++;
    lista->index ++;
}

void update(jobList * lista){
    int i ;
    int eliminados = 0;
    for (i = 0; i < lista->index; i++) {
        if (lista.listaJobs[i].delete) {
            lista.listaJobs[i] == NULL;
            eliminados ++;
            move(lista, i);
        }
    }
    lista->index -= eliminados;
}

void move(jobList * lista, int i){
    for (i; i < lista->index; i++){
        lista.listaJobs[i] = lista.listaJobs[i+1]
    }
}

void freeJobs(jobList * lista){
    free(lista->listaJobs->pids);
    free(lista.listaJobs);
    free(lista);
}

int isIn(pid_t pid, jobList* lista){
    int i;
    for (i = 0; i < lista.maximo; i++){
        if (pid == lista.listaJobs[i].lastPid){
            return 1;
        }
    }
    return 0;
}

int getContador(jobList * lista){
    return lista->contador;
}

tJob * getFirstJob(jobList * lista){
    return lista->listaJobs[0];
}

void deleteJob(tJob * job, jobList * lista){
    int i;
    for (i = 0; i < lista->index; i++ ){
        if (job == lista->listaJobs[i]){
            lista->listaJobs[i].delete = 1;
        }
    }
    update(lista);
}

tJob * getJobByPid(pid_t pid, jobList * lista){
    int i;
    for (i = 0; i < lista->index; i++ ){
        if (pid == lista->listaJobs[i].lastPid){
            return lista->listaJobs[i];
        }
    }
    return NULL;
}

pid_t * killPids(jobList * lista, int i){
    lista->listaJobs[i].delete = 1;
    return lista->listaJobs[i]->pids;
}

pid_t * getPids(tJob * job){
    return job.pids;
}

int getIndex(jobList * lista){
    return lista->index;
}
