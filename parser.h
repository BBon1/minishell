
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

extern tline * tokenize(char *str);

