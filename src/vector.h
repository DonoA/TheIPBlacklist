#pragma once

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>

typedef struct {
    uint8_t * data;
    size_t element_size;
    size_t used;
    size_t len;
} vector_t;

vector_t * newVector(size_t element_size);

void vectorAdd(vector_t * vec, void * data);

void * vectorGet(vector_t * vec, size_t i);

void deleteVector(vector_t * vec);