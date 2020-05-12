#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <malloc.h>

void beingMemoryProfiling();

void * profiledCalloc(size_t n, size_t size);

void profiledFree(void * ptr);

void endProfiling(size_t * allocated_out, size_t * freed_out);