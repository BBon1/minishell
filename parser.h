
typedef struct {
	char * filename;
	int argc;
	char ** argv;
} tcommand;

typedef struct {
	int ncommands;
	tcommand * commands;
	char * redirect_input;
	char * redirect_output;
	char * redirect_error;
	int background;
} tline;

typedef struct {
	int tuberia[2];
} tPipe;

typedef struct {
	int index;
	int pid;
	int status;
	char name[100];
	int view;  // 'Booleano' Saber si eliminar de la lista
} tJob;

extern tline * tokenize(char *str);

