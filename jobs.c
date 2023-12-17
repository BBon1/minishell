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


void initJobs(jobList nuevaLista){
    nuevaLista.contador = 1;
    nuevaLista.index = 0;
    nuevaLista.maximo = 10;
}

void initJob(tJob  nuevo ,int in, pid_t last, pid_t * listaPids, int st, char * n){
    int i ;
    nuevo.index = in;
    nuevo.lastPid = last;
    for (i = 0; i < 5; i++){
    	nuevo.pids[i] = listaPids[i];
    }
    nuevo.status = st;
    strcpy(nuevo.name, n);
    nuevo.delete = 0;
}

void initJobNoStatus(tJob  nuevo ,int in, pid_t last, pid_t * listaPids , char * n){
    int i ;
    nuevo.index = in;
    nuevo.lastPid = last;
    for (i = 0; i < 5; i++){
    	nuevo.pids[i] = listaPids[i];
    }
    strcpy(nuevo.name, n);
    nuevo.delete = 0;
}

int isFull(jobList lista){
    return lista.index == lista.maximo;
}


void printJobs(jobList lista){
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

void addJob(tJob newJob, jobList lista){
    lista.listaJobs[lista.index] = newJob;
    lista.contador ++;
    lista.index ++;
}

void update(jobList lista){
    int i ;
    int eliminados = 0;
    for (i = 0; i < lista.index; i++) {
        if (lista.listaJobs[i].delete) {
            &lista.listaJobs[i] == NULL;
            eliminados ++;
            move(lista, i);
        }
    }
    lista.index -= eliminados;
}

void move(jobList lista, int i){
    for (i; i < lista.index; i++){
        lista.listaJobs[i] = lista.listaJobs[i+1];
    }
}

int isIn(pid_t pid, jobList lista){
    int i;
    for (i = 0; i < lista.maximo; i++){
        if (pid == lista.listaJobs[i].lastPid){
            return 1;
        }
    }
    return 0;
}

int getContador(jobList lista){
    return lista.contador;
}

tJob getFirstJob(jobList lista){
    return lista.listaJobs[0];
}

void deleteJob(tJob job, jobList lista){
    int i;
    for (i = 0; i < lista.index; i++ ){
        if (job.index == lista.listaJobs[i].index){
            lista.listaJobs[i].delete = 1;
        }
    }
    update(lista);
}

tJob getJobByPid(pid_t pid, jobList lista){
    int i;
    tJob aux;
    aux.index = -1;
    for (i = 0; i < lista.index; i++ ){
        if (pid == lista.listaJobs[i].lastPid){
            return lista.listaJobs[i];
        }
    }
    return aux;
}

void killPids(jobList lista, pid_t pids [],int i){
    int o ;
    for (o = 0; o < 5; o++){
    	pids[o] = lista.listaJobs[i].pids[o];
    }
    lista.listaJobs[i].delete = 1;
}

void getPids(tJob job, pid_t pids []){
    int i ;
    for (i = 0; i < 5; i++){
    	pids[i] = job.pids[i];
    }
}

int getIndexJob(tJob job){
    return job.index;
}

int getIndex(jobList lista){
    return lista.index;
}

pid_t getLastPid(tJob job){
    return job.lastPid;
}
