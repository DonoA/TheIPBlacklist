#include "blacklist.h"
#include "allocator.h"
#include "common.h"
#include <assert.h>

#define EMPTY 0
#define DELETED 255

struct set_struct
{
    uint32_t *table;
    size_t table_len;
    size_t table_used;
};

set_t *newSet(size_t len)
{
    set_t *set = profiledCalloc(1, sizeof(set_t));
    set->table_len = len + 1;
    set->table_used = 0;
    set->table = profiledCalloc(len, sizeof(uint32_t));

    return set;
}

void deleteSet(set_t *set)
{
    profiledFree(set->table);
    profiledFree(set);
}

// Seems to be pretty good
// https://github.com/skeeto/hash-prospector
size_t hash(uint32_t x)
{
    x ^= x >> 16;
    x *= UINT32_C(0x7feb352d);
    x ^= x >> 15;
    x *= UINT32_C(0x846ca68b);
    x ^= x >> 16;
    return x;
}

void addIP(uint32_t address, void *context)
{
    set_t *set = (set_t *)context;
    assert(set->table_used < set->table_len);
    size_t adr_hash = hash(address);
    while (true)
    {
        uint32_t *elt = set->table + (adr_hash % set->table_len);
        if (*elt == DELETED || *elt == EMPTY)
        {
            *elt = address;
            set->table_used++;
            return;
        }

        adr_hash++;
    }
}

void setAddAll(set_t *set, subnet_t addresses)
{
    iterateIPs(&addresses, set, addIP);
}

bool setContains(set_t *set, uint32_t address)
{
    size_t adr_hash = hash(address);
    while (true)
    {
        uint32_t *elt = set->table + (adr_hash % set->table_len);
        if (*elt == address)
        {
            return true;
        }

        if (*elt == EMPTY)
        {
            return false;
        }

        adr_hash++;
    }
}