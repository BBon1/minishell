

typedef struct {
	int pid;
	int status;
	char name[100];
} trabajo;



void printJobs();
void addJob(int pid, char*name, int j);
void deleteJob();
int getPID(job);
void freeJobs();
int pidInJobs();
