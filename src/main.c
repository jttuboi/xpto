#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

#include "vector.h"
#include "tokenize.h"
#include "process.h"
#include "process_operations.h"

#include "type.h"

vector *jobs;
shell_conf* shell;

void signal_handler(int signal);

/* procedimento que coloca um comando externo em execucao */
void execute_command(vector *tokens, vector *job_vector, shell_conf *shell, char *path, char **envp) {
	int i;
  char *cmd;
  vector *paths;
  char *abs_path;

	job *j = new_job(tokens);
	
	BOOL foreground = is_foreground(tokens);
	
	if (!foreground) {
		erase(tokens, tokens->size - 1);
	}
	
	pid_t pid = fork ();
	if (pid == 0) {

		signal(SIGINT, SIG_DFL); //caso nao haja job em foreground, nao faz nada
		signal(SIGTSTP, SIG_DFL);
		signal(SIGTTIN, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		signal(SIGCHLD, signal_handler); 

		pid = getpid ();
		setpgrp();
		if (!j->pgid) 
			j->pgid = pid; /* o id do grupo de processos do job e' o id do processo lider */
		setpgid (pid, j->pgid);
		
		if (foreground) tcsetpgrp (shell->descriptor, pid); /* passa o controle de E/S para o novo processo */
		
		//Primeiro tenta-se executar o comando passado considerando que ele já possui o caminho na string:
    execve((char*)(tokens->content[0]), (char**)(tokens->content), envp);
		
    //Se execve terminou sem sucesso, o programa prossegue, caso contrário, após ele a imagem de outro programa
    //estará sendo executada ao invés dessa
    
    //Divide o PATH em substrings com os caminhos para tentar executar o programa associado a elas;	
    paths = split_string(path, ":");
    
    cmd = (char*)(tokens->content[0]);
    abs_path = (char *) malloc(100*sizeof(char));
		
    for(i = 0; i < paths->size; i++) {
      sprintf(abs_path, "%s/%s", (char *)element(paths, i), cmd);
      execve(abs_path, (char**)(tokens->content), envp);
    }
		
    printf("-B1: %s: command not found\n", (char*)tokens->content[0]);
    exit(1);
	} else if (pid < 0) {
		perror ("fork");
		exit (1);
	} else {
	    
	  push_back(j, job_vector); /* insercao do job na lista de jobs */
			    
		j->process->pid = pid;
		if (!j->pgid)
			j->pgid = pid;
		setpgid (pid, j->pgid);
		if (foreground) { /* se o processo estiver em foreground */
			launch_foreground(jobs, j, shell);	 /* inicia o processo */
      delete_job(job_vector, j->jid);		 /* remove o job da lista de jobs */
		} else { 
			launch_background(j, shell); /* inicia o processo em background */
		}
	}
	
		
}



/* configura alguns parametros do shell e armazena em uma estrutura para uso posterior */
shell_conf *init_shell() {
	shell_conf *sc = (shell_conf*)malloc(sizeof(shell_conf));
	
	signal(SIGINT, SIG_IGN); //caso nao haja job em foreground, nao faz nada
	signal(SIGTSTP, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN); /* necessario para o comando tcsetpgrp. (ver manual) */
	signal(SIGCHLD, signal_handler); 

		
	sc->pid = getpid();
	sc->descriptor = STDIN_FILENO; /* descritor de entrada e saida */

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

	printf("JID\tPID \tSTATE     \tCOMMAND\n");
	printf("---------------------------------------\n");	
	
	for (i = 0; i < jobs->size; i++) {
		job *j = (job *) jobs->content[i];
	
		printf("[%d]\t%d\t", j->jid, j->process->pid);

		/* impressao do status de cada job */
		if (j->status == FOREGROUND) {
			printf("running   ");
		} else if (j->status == BACKGROUND) {
			printf("running   ");
		} else if (j->status == STOPPED) {
			printf("stopped   ");
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

job *job_from_pid(pid_t pid) {
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
		printf("b1: %d: no such pid\n", pid);
		return NULL;
	} else {
		return ((job*)element(jobs, i-1));
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
		printf("b1: %d: no such job\n", jid);
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


void signal_handler(int p)
{
	pid_t pid;
	int status;
  pid = waitpid(WAIT_ANY, &status, WUNTRACED | WNOHANG);

	if (pid > 0) {
  	job* j = job_from_pid(pid);
		printf("command: %s\njid: %d\npid: %d\nstatus: %d\n", j->process->argv[0], j->jid, j->process->pid, j->status);

    if (j == NULL)
    	return;

    if (WIFEXITED(status)) {
			printf("Exited\n");
    	if (j->status == BACKGROUND) {
      	printf("\n[%d]+  Done\t   %s\n", j->jid, j->process->argv[0]);
        delete_job(jobs, j->jid);
      } 
		} else if (WIFSIGNALED(status)) {
			printf("Signaled\n");
    	printf("\n[%d]+  Killed\t   %s\n", j->jid, j->process->argv[0]);
      delete_job(jobs, j->jid);
    } else if (WIFSTOPPED(status)) {
    	if (j->status == BACKGROUND) {
				printf("Stopped back\n");
      	tcsetpgrp(shell->descriptor, shell->pgid);
        j->status = WAITING_INPUT;
        printf("\n[%d]+   suspended [wants input]\t   %s\n", j->jid, j->process->argv[0]);
			} else {
				printf("Stopped not back\n");
				tcsetpgrp(shell->descriptor, shell->pgid);
        j->status = STOPPED;
				//push_back(j, jobs);
        printf("\n[%d]+   stopped\t   %s\n", j->jid, j->process->argv[0]);
      }
      return;
    } else {
			printf("Others\n");
    	if (j->status == BACKGROUND) {
				printf("Others back\n");
      	delete_job(jobs, j->jid);
      }
    }
    tcsetpgrp(shell->descriptor, shell->pgid);
  }
}

int main (int argc, char** argv, char** envp) {
  BOOL quit;
  char *command = (char*)malloc(100*sizeof(char));
  char *path = getenv("PATH");
  vector *tokens;
	char is_jid;
	pid_t jid;
	jobs = new_vector();

	shell = init_shell();

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
							job *j = (job_from_pid(atoi(element(tokens, 1))));
							if (j != NULL) {
								jid = j->jid;
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
