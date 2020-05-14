#include "blacklist.h"
#include "lib/allocator.h"
#include "lib/common.h"
#include <assert.h>

#define EMPTY 0

struct set_struct
{
    uint32_t *table;
    size_t table_len;
    size_t table_used;
 
    size_t total_search_dist;
    size_t search_count;
    size_t search_over_cache_line;
};

set_t *newSet(size_t len, float load_factor)
{
    set_t *set = profiledCalloc(1, sizeof(set_t));
    set->table_len = len * load_factor;
    set->table_used = 0;
    set->table = profiledCalloc(set->table_len, sizeof(uint32_t));

    set->total_search_dist = 0;
    set->search_count = 0;
    set->search_over_cache_line = 0;

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
    x ^= x >> 17;
    x *= UINT32_C(0xed5ad4bb);
    x ^= x >> 11;
    x *= UINT32_C(0xac4c1b51);
    x ^= x >> 15;
    x *= UINT32_C(0x31848bab);
    x ^= x >> 14;
    return x;
}

void addIP(uint32_t address, void *context)
{
    if(address == EMPTY)
    {
        return;
    }

    set_t *set = (set_t *)context;
    assert(set->table_used < set->table_len);
    size_t adr_hash = hash(address);
    size_t checked = 0;
    while (checked < set->table_len)
    {
        uint32_t *elt = set->table + (adr_hash % set->table_len);
        if (*elt == address)
        {
            return;
        }
        
        if (*elt == EMPTY)
        {
            *elt = address;
            set->table_used++;
            return;
        }

        checked++;
        adr_hash++;
    }

    char * adr = addressAsStr(address);
    printf("Failed while adding (checked %li) %s\n", checked, adr);
    free(adr);
    exit(1);
}

void setAddAll(set_t *set, subnet_t addresses)
{
    iterateIPs(&addresses, set, addIP);
}

bool setContains(set_t *set, uint32_t address)
{
    if(address == EMPTY)
    {
        return true;
    }

    set->search_count++;
    size_t adr_hash = hash(address);
    size_t checked = 0;
    while (checked < set->table_len)
    {
        if(checked % 16 == 0)
        {
            set->search_over_cache_line++;
        }
        
        uint32_t *elt = set->table + (adr_hash % set->table_len);
        if (*elt == address)
        {
            return true;
        }

        if (*elt == EMPTY)
        {
            return false;
        }

        set->total_search_dist++;
        checked++;
        adr_hash++;
    }

    char * adr = addressAsStr(address);
    printf("Failed while checking (checked %li) %s\n", checked, adr);
    free(adr);
    exit(1);
}

size_t setGetSize(set_t * set)
{
    return set->table_used;
}

void setPrintExtraStats(set_t * set)
{
    printf("Average Search Dist = %f\n", ((double) set->total_search_dist/set->search_count));
    printf("Searches over 16 elts = %lu\n", set->search_over_cache_line - set->search_count);
}