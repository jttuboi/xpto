
#ifndef _PROCESS_H_ 
#define _PROCESS_H_

#include "type.h"

typedef struct process {
	struct process *next;
  char **argv;                /* argumentos a serem passados para o processo */
  pid_t pid;                  /* process ID */
  char completed;             /* true se processo tiver terminado */
  char stopped;               /* true se processo estiver 'parado' */
  //int status;                 /* valor do status */
} process;

typedef struct {
  process *process;           /* processo nesse job */
  pid_t pgid;                 /* process group ID */
	pid_t jid;									/* job id */
  //char notified;              /* true se usuario pediu para parar o job */
} job;


job *new_job(vector *tokens) {
	static int next_jid = 1;

  process *p = (process *)malloc(sizeof(process));
  job *j = (job *)malloc(sizeof(job));
  
  j->process = p;
	j->jid = next_jid++;

  p->argv = (char **)tokens->content;
  p->stopped = FALSE;
  p->completed = FALSE;
	p->next = NULL;
	p->pid = 0;
	
	return j;
}

/*
// busca um job ativo com o pgid dado.
job *find_job (pid_t pgid) {
  job *j;
     
  for (j = first_job; j != NULL; j = j->next)
    if (j->pgid == pgid)
      return j;
  
  return NULL;
}
     
// Return true if all processes in the job have stopped or completed.
BOOL job_is_stopped (job *j) {
  process *p;
     
  for (p = j->first_process; p != NULL; p = p->next)
    if (!p->completed && !p->stopped)
      return FALSE;
  return TRUE;
}

// Return true if all processes in the job have completed.
BOOL job_is_completed (job *j) {
  process *p;
     
  for (p = j->first_process; p != NULL; p = p->next)
    if (!p->completed)
      return FALSE;
  return TRUE;
}*/

#endif
