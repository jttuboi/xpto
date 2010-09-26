
#ifndef _PROCESS_H_ 
#define _PROCESS_H_

#include "type.h"

typedef struct {
  struct process *next;       /* proximo processo na pipeline */
  char **argv;                /* argumentos a serem passados para o processo */
  pid_t pid;                  /* process ID */
  char completed;             /* true se processo tiver terminado */
  char stopped;               /* true se processo estiver 'parado' */
  int status;                 /* valor do status */
} process;

typedef struct job {
  struct job *next;           /* proximo job ativo */
  //char *command;              /* command line, used for messages */
  process *first_process;     /* lista dos processos nesse job */
  pid_t pgid;                 /* process group ID */
  char notified;              /* true se usuario pediu para parar o job */
  //struct termios tmodes;      /* saved terminal modes */
  int stdin, stdout, stderr;  /* canais padrao de E/S */
} job;

/* busca um job ativo com o pgid dado. */
job *find_job (pid_t pgid) {
  job *j;
     
  for (j = first_job; j != NULL; j = j->next)
    if (j->pgid == pgid)
      return j;
  
  return NULL;
}
     
/* Return true if all processes in the job have stopped or completed.  */
BOOL job_is_stopped (job *j) {
  process *p;
     
  for (p = j->first_process; p; p = p->next)
    if (!p->completed && !p->stopped)
      return FALSE;
  return TRUE;
}

/* Return true if all processes in the job have completed.  */
BOOL job_is_completed (job *j) {
  process *p;
     
  for (p = j->first_process; p != NULL; p = p->next)
    if (!p->completed)
      return FALSE;
  return TRUE;
}


#endif
