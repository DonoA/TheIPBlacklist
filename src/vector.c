#include "vector.h"

vector_t * newVector(size_t element_size)
{
    vector_t * v = (vector_t *) calloc(1, sizeof(vector_t));
    assert(v != NULL);

    v->used = 0;
    v->len = 16;
    v->element_size = element_size;
    v->data = (uint8_t *) calloc(v->len, v->element_size);

    return v;
}

void vectorAdd(vector_t * vec, void * data)
{
    if(vec->len <= vec->used) 
    {
        void * oldData = vec->data;
        vec->data = (uint8_t *) calloc(vec->len * 2, vec->element_size);
        memcpy(vec->data, oldData, vec->element_size * vec->used);
        free(oldData);
        vec->len *= 2;
    }

    void * newDataSlot = vec->data + (vec->used * vec->element_size);
    memcpy(newDataSlot, data, vec->element_size);
    vec->used++;
}

void * vectorGet(vector_t * vec, size_t i)
{
    return (void *) (vec->data + (i * vec->element_size));
}

void deleteVector(vector_t * vec)
{
    free(vec->data);
    free(vec);
}