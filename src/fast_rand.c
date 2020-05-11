#include "fast_rand.h"

size_t fastRandSeed = 1000;

size_t fastRand()
{
    fastRandSeed = ((fastRandSeed * 1103515245) + 12345) * 31;
    return fastRandSeed;
}