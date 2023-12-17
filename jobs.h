
#include <sys/types.h>
#include <unistd.h>


typedef struct {
    int index;
    pid_t lastPid;
    pid_t pids[5];
    int status;
    char name[100];
    int delete;  // 'Booleano' Saber si eliminar de la lista
} tJob;

typedef struct {
    int contador;
    int index;
    int maximo;
    tJob listaJobs[5];
} jobList;

jobList initJobs();
tJob initJob(int in, pid_t last, pid_t * listaPids, int st, char * n);
tJob initJobNoStatus(int in, pid_t last, pid_t * listaPids , char * n);
int isFull(jobList lista);
// void reallocJobs(jobList lista);
void printJobs(jobList lista);
jobList addJob(tJob  newJob, jobList lista);
jobList update(jobList lista);
jobList clean(jobList lista);
jobList deleteJob(tJob  job, jobList lista);
// void freeJobs(jobList lista);
tJob getFirstJob(jobList lista);
int getContador(jobList lista);
int isIn(pid_t pid, jobList lista);
jobList move(jobList lista, int i);
tJob getJobByPid(pid_t pid, jobList lista);
pid_t killPids (jobList lista, pid_t pids [],int i);
pid_t getPids(tJob  job, pid_t pids []);
int getIndex(jobList lista);
pid_t getLastPid(tJob job);
int getIndexJob(tJob job);


