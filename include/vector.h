#ifndef _VECTOR_H_
#define _VECTOR_H_

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    void** content;
    size_t size;  
    size_t max;
} vector;

void push_back(void* element, vector* v);

size_t size(vector* v);

int empty(vector* v);

void* element(vector* v, size_t pos);

void* front(vector* v);

void* back(vector* v);

void insert(vector* v, size_t pos, void* el);

void erase(vector* v, size_t pos, void* el);

vector* new_vector();

void delete_vector(vector* v);

#endif