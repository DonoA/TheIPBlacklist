#include "allocator.h"
#include <stdint.h>
#include <string.h>

bool allocator_profiler_running = false;
size_t allocator_allocated = 0;
size_t allocator_freed = 0;

void beingMemoryProfiling()
{
    allocator_profiler_running = true;
    allocator_allocated = 0;
    allocator_freed = 0;
}

void * profiledCalloc(size_t n, size_t size)
{
    if(allocator_profiler_running) 
    {
        allocator_allocated += (size * n);
    }

    uint8_t * space = (uint8_t *) aligned_alloc(64, n * size);
    memset(space, 0, n * size);
    return space;
}

void profiledFree(void * ptr)
{
    if(allocator_profiler_running) 
    {
        // This is a gnu extension, check https://stackoverflow.com/a/1281720 for additional implementations
        allocator_freed += malloc_usable_size(ptr);
    }
    free(ptr);
}

void endProfiling(size_t * allocated_out, size_t * freed_out)
{
    *allocated_out = allocator_allocated;
    *freed_out = allocator_freed;
    allocator_profiler_running = false;
}