




void printJobs (int i){
	char s [200];		 
	if (trabajo.pid == waitpid(trabajo.pid, NULL , WNOHANG)){  
	// Ha terminado por su cuenta
		if (WEXITSTATUS(trabajo.status) == 0){
			strcpy(s, "Done");
		}
	} else if (waitpid(trabajo..pid, NULL , WNOHANG) == -1) { // Error
		strcpy(s, "Error"); 
	} else { // No ha terminado
		if (WIFSTOPPED(trabajo.status)){
			strcpy(s, "Stopped");
		} else if (WIFSIGNALED(trabajo.status)){
			strcpy(s, "Signaled");
		} else {
			strcpy(s, "Running");
		}
	}					
		fprintf(stdout,"[%d] %s       %s", i+1 , s, trabajo.name);
}


void addJob( pid, name, j){
	trabajo auxJ;
	
	auxJ.pid = hijos[line->ncommands-1]; 
	strcpy(auxJ.name, name);
	jobs_array[j] = auxJ;
	j ++;
	
}

void deleteJob() {


}


int getLastPID(job){
	return trabajo.name
}



