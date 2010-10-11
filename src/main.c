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


/* configura alguns parametros do shell e armazena em uma estrutura para uso posterior */
shell_conf *init_shell() {
	shell_conf *sc = (shell_conf*)malloc(sizeof(shell_conf));

		
	sc->pid = getpid();
	sc->descriptor = STDIN_FILENO; /* descritor de entrada e saida */

	signal(SIGTTOU, SIG_IGN); /* necessario para o comando tcsetpgrp. (ver manual) */
	setpgid(sc->pid, sc->pid);
	sc->pgid = getpgrp(); /* recupera o id do grupo de processos em foreground */
	if (sc->pid != sc->pgid) {
		printf("Error, the shell is not process group leader");
		exit(EXIT_FAILURE);
	}	
	
	tcsetpgrp(sc->descriptor, sc->pgid); /* passa descritor de entrada para o grupo do shell */

	return sc;
}


/* comando jobs. Lista todos os jobs do shell */
void show_jobs(vector *jobs) { 
	int i;

	for (i = 0; i < jobs->size; i++) {
		job *j = (job *) jobs->content[i];
		
		printf("[%d]\t", j->jid);
		
		/* impressao do status de cada job */
		if (j->status == FOREGROUND) {
			printf("foreground");
		} else if (j->status == BACKGROUND) {
			printf("background");
		}

		printf("\t%s\n", j->process->argv[0]);
	}

}


/* implementacao do comando interno cd */
void change_directory(vector *tokens) {

	if (tokens->size == 1) {
		chdir(getenv("HOME")); /* cd passado sem um caminho */
	} else { 
		if (chdir((char *) tokens->content[1]) == -1) { /* tenta mudar de diretorio */
			printf("%s: no such directory.\n", (char *) tokens->content[1]); /* diretoria nao encontrado */
		}
	}

}

pid_t jid_from_pid(pid_t pid, vector *jobs) {
	BOOL found = 0;
	int i = 0;
	process *p;

	while(i < jobs->size) {
		p = ((job *)element(jobs, i))->process;
		while (!found && p != NULL) {
			if (p->pid == pid)
				found = 1;
			p = p->next;
		}
		i++;
	}


	if (!found) {
		printf("fg: %d: no such pid\n", pid);
		return -1;
	} else {
		return ((job*)element(jobs, i-1))->jid;
	}

}

/* comando para passar um job para foreground */
void fg(pid_t jid, vector *jobs, shell_conf *shell) {
	BOOL found = 0;
	int i = 0;

	/* percorre a lista ate encontrar */
	while (!found && i < jobs->size) {
		if (((job *)element(jobs, i))->jid == jid) {
			put_foreground((job *)element(jobs, i), shell); /* coloca o job em foreground */
			delete_job(jobs, jid); /* remove o job ja terminado da lista de jobs */
			found = 1;
		}
		i++;
	}
	
	if (!found)  /* caso nao exista um job com o id dado */
		printf("fg: %d: no such job\n", jid);
}

/* move um processo para background */
void bg(pid_t jid, vector *jobs, shell_conf *shell) {
	BOOL found = 0;
	int i = 0;

	/* percorre a lista de jobs */
	while (!found && i < jobs->size) {
		if (((job *)element(jobs, i))->jid == jid) {
			put_background((job *)element(jobs, i), shell);
			found = 1;
		}
		i++;
	}
	
	if (!found) 
		printf("bg: %d: no such job\n", jid);
}


/* implementacao do comando pwd */
void show_path() {
	printf("%s\n", getcwd(NULL, 0));
}

int main (int argc, char** argv, char** envp) {
  BOOL quit;
  char *command = (char*)malloc(100*sizeof(char));
  char *path = getenv("PATH");
  vector *tokens;
  vector *jobs = new_vector();
	char is_jid;
	pid_t jid;

  shell_conf *shell = init_shell();
  quit = FALSE;
  while (!quit) {
		printf("%s B1> ",getcwd(NULL, 0));
		fgets(command, 100, stdin);
    		
    tokens = split_string(command, " \n");
       
	 /* procura por comandos internos */	
    if (tokens->size != 0) {
			if (strcmp(element(tokens, 0), "exit") == 0 || strcmp(element(tokens, 0), "quit") == 0) {
				quit = TRUE;
			} else if (strcmp(element(tokens, 0), "jobs") == 0) {
				show_jobs(jobs);
			} else if (strcmp(element(tokens, 0), "fg") == 0) {
				if (tokens->size == 1) {
					printf("fg: need a job id\n");
				} else if (tokens->size > 2) {
					printf("fg: unexpected parameter %s\n", (char *) element(tokens, 2));
				} else {
						is_jid = ((char* )element(tokens, 1))[0];
						if (is_jid == '%') {
								fg(atoi(element(tokens, 1) + 1), jobs, shell);
						} else {
							jid = jid_from_pid(atoi(element(tokens, 1)), jobs);
							if (jid != -1) {
								fg(jid, jobs, shell);	
							}
						}	
				}
			} else if (strcmp(element(tokens, 0), "bg") == 0) {
				if (tokens->size == 1) {
					printf("bg: need a job id\n");
				} else if (tokens->size > 2) {
					printf("bg: unexpected parameter %s\n", (char *) element(tokens, 2));
				} else {
					bg(atoi(element(tokens, 1)), jobs, shell);
				}
			} else if (strcmp(element(tokens, 0), "cd") == 0) {
				change_directory(tokens);
			} else if (strcmp(element(tokens, 0), "pwd") == 0) {
				show_path();
			} else { // demais comandos
				execute_command(tokens, jobs, shell, path, envp);
			}
    }
	}    

	
  printf("logout\n\n[Process completed]\n");
    
  return 0;
}
