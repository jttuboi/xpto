#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "vector.h"
#include "tokenize.h"
#include "process.h"
#include "process_operations.h"

#include "type.h"

shell_conf *init_shell() {
	shell_conf *sc = (shell_conf*)malloc(sizeof(shell_conf));
	
	sc->pid = getpid();
	sc->descriptor = STDIN_FILENO;
	sc->is_interactive = isatty(sc->descriptor);

	//if (sc->is_interactive) { 
	//	while(tcgetpgrp(sc->descriptor) != (sc->pgid = getpgrp()))
	//		kill(sc->pid, SIGTTIN);
	
	
		signal(SIGTTOU, SIG_IGN);
		setpgid(sc->pid, sc->pid);
		sc->pgid = getpgrp();
		if (sc->pid != sc->pgid) {
			printf("Error, the shell is not process group leader");
			exit(EXIT_FAILURE);
		}	
	
		tcsetpgrp(sc->descriptor, sc->pgid);
	//}

	return sc;
}

void show_jobs(vector *jobs) { 
	int i;

	for (i = 0; i < jobs->size; i++) {
		job *j = (job *) jobs->content[i];
		printf("[%d]\tStatus\t%s\n", j->jid, (char *)j->process->argv[0]);
	}

}

void change_directory(vector *tokens) {

	if (tokens->size == 1) {
		chdir(getenv("HOME"));
	} else { 
		if (chdir((char *) tokens->content[1]) == -1) {
			printf("%s: no such directory.\n", (char *) tokens->content[1]);
		}
	}

}

int main (int argc, char** argv, char** envp) {
  BOOL quit;
  char *command = (char*)malloc(100*sizeof(char));
  char *path = getenv("PATH");
  vector *tokens;
  vector *jobs = new_vector();

	shell_conf *shell = init_shell();
    
  quit = FALSE;
  while (!quit) {
    printf("B1> ");
		fgets(command, 100, stdin);
    
    tokens = split_string(command, " \n");
        
    if (tokens->size != 0) {
			if (strcmp(element(tokens, 0), "exit") == 0 || strcmp(element(tokens, 0), "quit") == 0) {
				quit = TRUE;
			} else if (strcmp(element(tokens, 0), "jobs") == 0) {
				show_jobs(jobs);
			} else if (strcmp(element(tokens, 0), "cd") == 0) {
				change_directory(tokens);
			} else if (strcmp(element(tokens, 0), "pwd") == 0) {
				//show_path();
			} else {
				execute_command(tokens, jobs, shell, path, envp);
			}
    }
    
    delete_vector(tokens);       
  }    

  printf("logout\n\n[Process completed]\n");
    
  return 0;
}
