#ifndef _PROCESS_OPERATIONS_H_
#define _PROCESS_OPERATIONS_H_

#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "vector.h"
#include "tokenize.h"
#include "process.h"

#include "type.h"

/*int execute(vector* tokens, char *path, char **envp) {
  int i;
  char *cmd;
  vector *paths;
  char *abs_path;
  int success;
	
  if (fork() == 0) {
    //Primeiro tenta-se executar o comando passado considerando que ele já possui o caminho na string:
    execve((char *)element(tokens, 0), (char**)(tokens->content), envp);
		
    //Se execve terminou sem sucesso, o programa prossegue, caso contrário, após ele a imagem de outro programa
    //estará sendo executada ao invés dessa
    
    //Divide o PATH em substrings com os caminhos para tentar executar o programa associado a elas;	
    paths = split_string(path, ":");
    
    cmd = (char *) element(tokens, 0);
    abs_path = (char *) malloc(100*sizeof(char));
		
    for(i = 0; i < paths->size; i++) {
      sprintf(abs_path, "%s/%s", (char *)element(paths, i), cmd);
      execve(abs_path, (char**)(tokens->content), envp);
    }
		
    free(abs_path);
    //caso o programa tenha chegado até aqui, nao houve sucesso na execucao do programa junto com o PATH, então ele
    //sem sucesso:
    printf("-B1: %s: command not found\n", (char*)tokens->content[0]);
    exit(1);
  } else {
    //espera acabar: http://www.opengroup.org/onlinepubs/009695399/functions/wait.html
    wait(&success);
  }
	
  //retorna se o programa filho executou com sucesso (convencionado 1 para sucesso)
  return !success;
}*/

BOOL is_foreground(vector *command) {
  return strcmp(back(command), "&");
}


void execute_command(vector *tokens, vector *job_vector, shell_conf *shell, char *path, char **envp) {
	int i;
  char *cmd;
  vector *paths;
  char *abs_path;
		
	job *j = new_job(tokens);
	push_back(j, job_vector);
	
	BOOL foreground = is_foreground(tokens);
	
	//if (!foreground) {
	//	erase(tokens, tokens->size - 1);
	//}
	
	pid_t pid = fork ();
	if (pid == 0) {

		pid = getpid ();
		if (!j->pgid) 
			j->pgid = pid;
		setpgid (pid, j->pgid);
		
		if (foreground) tcsetpgrp (shell->descriptor, pid);
		
		//Primeiro tenta-se executar o comando passado considerando que ele já possui o caminho na string:
    execve(j->process->argv[0], (char**)(tokens->content), envp);
		
    //Se execve terminou sem sucesso, o programa prossegue, caso contrário, após ele a imagem de outro programa
    //estará sendo executada ao invés dessa
    
    //Divide o PATH em substrings com os caminhos para tentar executar o programa associado a elas;	
    paths = split_string(path, ":");
    
    cmd = j->process->argv[0];
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
		j->process->pid = pid;
		if (!j->pgid)
			j->pgid = pid;
		setpgid (pid, j->pgid);
	}
	
	waitpid (j->process->pid, NULL, WUNTRACED);
	
	/* Put the job into the foreground.  */
	tcsetpgrp (shell->descriptor, j->pgid);
		
}


#endif
