#ifndef _PROCESS_OPERATIONS_H_
#define _PROCESS_OPERATIONS_H_

#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "vector.h"
#include "tokenize.h"
#include "process.h"

#include "type.h"

/* remove o job de id passado */
void delete_job(vector *jobs, pid_t id) {
  int i = 0;
  BOOL finished = FALSE;
  job *j;
  while (i < jobs->size && !finished) {
    j = (job *)jobs->content[i];
    if(j->jid == id) {
      erase(jobs, i);
      finished = TRUE;
    }
    i++;
  }
}

job *get_job(vector *jobs, pid_t id) {
  int i = 0;
  BOOL finished = FALSE;
  job* j = NULL;
  while (i < jobs->size && !finished) {
    j = (job *)jobs->content[i];
    if(j->jid == id) {
      j = jobs->content[i];
      finished = TRUE;
    }
    i++;
  }  
  return j;
}


BOOL is_foreground(vector *command) {
  return strcmp(back(command), "&");
}

/* procedimento que coloca um job em foreground */
void put_foreground(job *j, shell_conf *sc)
{
	j->status = FOREGROUND;
	tcsetpgrp(sc->descriptor, j->pgid); /* passa o descritor para o job */

	if (kill(-j->pgid, SIGCONT) < 0) { /* manda sinal para job continuar execucao */
		perror("kill (SIGCONT)\n");
	}

  waitpid(j->process->pid, NULL, WUNTRACED); /* espera o job terminar sua execucao */
  tcsetpgrp(sc->descriptor, sc->pgid); /* retorna o controle de E/S para o terminal */
}

/* coloca um processo em background */
void put_background(job *j, shell_conf *sc)
{
	if (j == NULL)
  	return;

	printf("[%d]\t%d\n", j->jid, j->process->pid);

  j->status = BACKGROUND;
	tcsetpgrp(sc->descriptor, sc->pgid);
}


void launch_background(job *j, shell_conf *sc)
{
  put_background(j, sc);
}

void launch_foreground(job *j, shell_conf *sc)
{
	j->status = FOREGROUND;
	tcsetpgrp(sc->descriptor, j->pgid);
  waitpid(j->process->pid, NULL, WUNTRACED);
  tcsetpgrp(sc->descriptor, sc->pgid);
}

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

		pid = getpid ();
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
			launch_foreground(j, shell);	 /* inicia o processo */
      delete_job(job_vector, j->jid);		 /* remove o job da lista de jobs */
		} else { 
			launch_background(j, shell); /* inicia o processo em background */
		}
	}
	
		
}


#endif
