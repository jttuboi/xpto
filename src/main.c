#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "vector.h"

vector* split_string(char*command, char* delimiters) {
// Retorna um vetor de tokens a partir da string comando, delimitados pelos elementos em delimiters
  vector* tokens = new_vector();
    
  char* token_position;
  char* token;
    
    //copia str para command para poder manipular
  char* str  = (char*)malloc(sizeof(char)*strlen(command));
  strcpy(str, command);
    
    //aloca memória para guardar o token encontrado
  token_position = strtok(str, delimiters);
  if (token_position != NULL) {
    token = (char*)malloc(sizeof(char)*strlen(token_position));
    strcpy(token, token_position);
  }
                    
  while (token_position != NULL) {
        //guarda o token no vetor de retorno
    push_back(token, tokens);
       
       //aloca memória para guardar o token encontrado
    token_position = strtok(NULL, delimiters);
    if (token_position != NULL) {
      token = (char*)malloc(sizeof(char)*strlen(command));
      strcpy(token, token_position);
    }
  }
        

    
  return tokens;
}

int execute(vector* tokens, char *path, char **envp) {
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
}

BOOL is_background(vector *command) {
  return !strcmp(back(command), "&");
}

int main (int argc, char** argv, char** envp) {
  int quit;
  char *command = (char*)malloc(100*sizeof(char));
  char *path = getenv("PATH");
  vector *tokens;
  vector *jobs = new_vector();
  job *j;
  process *p;
    
  quit = 0;
  while (!quit) {
    printf("B1> ");
    fgets(command, 100, stdin);
        
    tokens = split_string(command, " \n");
        
    if (tokens->size != 0) {
      if ((strcmp(element(tokens, 0), "exit") != 0)&&(strcmp(element(tokens, 0), "quit") != 0)) {
        
        j = new_job(tokens);

        jobs->push_back(j);
        
        if (is_background(tokens)) {  
        

          execute(j->process, path, envp);  
        } else {

          execute(p, path, envp);  
        }
      } else {
        quit = 1;
      }
    }
    
    delete_vector(tokens);       
  }    

  printf("logout\n\n[Process completed]\n");
    
  return 0;
}
