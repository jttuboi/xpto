#include "vector.h"

void push_back(void* element, vector* v) {
  if (v->size < v->max) {
    v->content[v->size++] = element;
  }
}

size_t size(vector* v) {
  return v->size;
}

int empty(vector* v) {
  return (v->size == 0);
}

void* element(vector* v, size_t pos) {
  if (v->size <= pos) {
    printf("ERRO em element: Acesso a posicao nao existente\n");
    exit(1);
  } else {
    return v->content[pos];
  }
}

void* front(vector* v) {
  if (empty(v)) {
    return NULL;
  } else {
    return v->content[0];
  }
}

void* back(vector* v) {
  if (empty(v)) {
    return NULL;
  } else {
    return v->content[v->size - 1];
  }
}

void insert(vector* v, size_t pos, void* el) {
  if (v->size < pos) {
    v->content[pos] = el;
  } else {
    printf("ERRO em insert: Acesso a posicao nao existente\n");
    exit(1);
  }
}

void erase(vector* v, size_t pos) {
  int i;
  if (v->size > pos) {
    free(v->content[pos]);
    for (i = pos; i < v->size - 1; i++)
      v->content[i] = v->content[i+1];
		v->content[--v->size] = NULL;
  } else {
    printf("ERRO em erase: Acesso a posicao nao existente\n");
    exit(1);
  }
}

vector* new_vector() {
  int i;
    
  vector* v = (vector*)malloc(sizeof(vector));
  v->content = (void**)malloc(100*sizeof(void*));
  v->max = 100;
  v->size = 0;
    
  for (i = 0; i < v->max; i++)
    v->content[i] = NULL;    
    
  return v;
}
 
void delete_vector(vector* v) {
  int i;

  if(v != NULL) {
    for(i = 0; i < v->size; i++) {
      if (v->content[i] != NULL) {
        free(v->content[i]);
      }
    }
    free(v);
  }
}
