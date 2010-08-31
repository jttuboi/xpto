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
        token_position = strtok(NULL, " \n");
        if (token_position != NULL) {
            token = (char*)malloc(sizeof(char)*strlen(command));
            strcpy(token, token_position);
        }
    }
    
    return tokens;
}

void execute(vector* tokens, char *path, char **envp) {
  	int e, i;
		vector *paths = split_string(path, ":");
		char *cmd = (char *) element(tokens, 0);
		char *abs_path = (char *) malloc(100*sizeof(char));

		i = 0; e = 0;
		if (fork() == 0) {
				while (i < paths->size) {
						strcat(abs_path, (char *)element(paths, i));
						strcat(abs_path, "/");
						strcat(abs_path, cmd);
        		e = execve(abs_path, (char**)(tokens->content), envp);
      			if ((e == -1)&&(tokens->content[0] != NULL)) {
        				i++;
    				} else {
       		 			//espera acabar: http://www.opengroup.org/onlinepubs/009695399/functions/wait.html
        				wait(NULL);
    				}
				}
		}
		if ( e == -1) { 
				printf("-B1: %s: command not found\n", (char*)tokens->content[0]);
    		exit(1);
		}

}

int main (int argc, char** argv, char** envp) {
    int quit = 0;
    char* command = (char*)malloc(100*sizeof(char));
		char *path = getenv("PATH");
    vector* tokens;
    
    while (!quit) {
        printf("\nB1> ");
        fgets(command, 100, stdin);
        
        tokens = split_string(command, " \n");
        
        execute(tokens, path, envp);        
        
        delete_vector(tokens);       
    }    

    
    return 0;
}
