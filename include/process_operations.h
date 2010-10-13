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

void launch_foreground(vector *jobs, job *j, shell_conf *sc)
{
	int status;
	j->status = FOREGROUND;
	tcsetpgrp(sc->descriptor, j->pgid);
  
	while (waitpid(j->process->pid, &status, WNOHANG) == 0) {
  	if (j->status == STOPPED)
  		return;
  }
  //delete_job(jobs, j->jid);
  tcsetpgrp(sc->descriptor, sc->pgid);
}

#endif
