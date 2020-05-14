#include "blacklist.h"

#include "lib/allocator.h"
#include "lib/common.h"

#define BLOCK_LEN 8

struct set_struct
{
    uint8_t *table;
    size_t size;
};

set_t *newSet(size_t _, float __)
{
    size_t ipRange = (1L << 32) - 1;
    set_t *set = profiledCalloc(1, sizeof(set_t));
    set->table = profiledCalloc(ipRange / BLOCK_LEN, sizeof(uint8_t));
    return set;
}

void deleteSet(set_t *set)
{
    profiledFree(set->table);
    profiledFree(set);
}

void addIP(uint32_t address, void *context)
{
    set_t *set = (set_t *)context;

    size_t offset = address / BLOCK_LEN;
    uint8_t *seg = set->table + offset;
    size_t mask = 1 << (address % BLOCK_LEN);
    if ((*seg & mask) == 0)
    {
        set->size++;
        *seg |= mask;
    }
}

void setAddAll(set_t *set, subnet_t addresses)
{
    iterateIPs(&addresses, set, addIP);
}

bool setContains(set_t *set, uint32_t address)
{
    size_t offset = address / BLOCK_LEN;
    uint8_t *seg = set->table + offset;
    size_t mask = 1 << (address % BLOCK_LEN);
    return (*seg & mask) != 0;
}

size_t setGetSize(set_t *set)
{
    return set->size;
}

void setPrintExtraStats(set_t * set)
{
    
}