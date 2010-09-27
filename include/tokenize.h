#ifndef _TOKENIZE_H_
#define _TOKENIZE_H_

#include <string.h>
#include <stdlib.h>

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

#endif