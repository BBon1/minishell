
typedef struct {
    int index;
    pit_t lastPid;
    pid_t * pids;
    int status;
    char name[100];
    int delete;  // 'Booleano' Saber si eliminar de la lista
} tJob;

typedef struct {
    int contador;
    int index;
    int maximo;
    tJob * listaJobs;
} jobList;

void initJobs(jobList * nuevaLista);
void initJob(tJob * nuevo, int in, pid_t last, pid_t * listaPids, int st, char * n);
void initJobNoStatus(tJob * nuevo ,int in, pid_t last, pid_t * listaPids, char * n);
int isFull(jobList * lista);
void reallocJobs(jobList * lista);
void printJobs(jobList * lista);
void addJob(tJob newJob, jobList * lista);
void update(jobList * lista);
void deleteJob(tJob job, jobList * lista);
void freeJobs(jobList * lista);
tJob * getFirstJob(jobList * lista);
int getContador(jobList * lista);
int isIn(pit_t pid, jobList* lista);
void move(jobList * lista, int i);
tJob* getJobByPid(pid_t pid, jobList * lista);
pid_t * killPids(jobList * lista, int i);
pid_t * getPids(tJob job);
int getIndex(jobList * lista);
