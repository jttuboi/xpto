#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

void set_path(vector* command, char** paths) {

}

void execute(vector* cmd, char**envp) {
    int e;
    if (fork() == 0) {
        e = execve((char*)cmd->content[0], (char**)(cmd->content), envp);
        if ((e < 0)&&(cmd->content[0] != NULL)) {
            printf("-B1: %s: command not found", (char*)cmd->content[0]);
            exit(1);
        }        
    } else {
        //espera acabar: http://www.opengroup.org/onlinepubs/009695399/functions/wait.html
        wait(NULL);
    }
}

int main (int argc, char** argv, char** envp) {
    int quit = 0;
    char* command = (char*)malloc(100*sizeof(char));
    vector* tokens;
    
    while (!quit) {
        printf("B1> ");
        fgets(command, 100, stdin);
        
        tokens = split_string(command, " \n");
        
        set_path(tokens, NULL);
        
        execute(tokens, envp);        
        
        delete_vector(tokens);       
    }    

    
    return 0;
}