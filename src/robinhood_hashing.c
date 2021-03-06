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
    size_t max_dist;

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
    set->max_dist = 0;

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

size_t displacementOf(set_t *set, size_t loc)
{
    return loc - (hash(set->table[loc]) % set->table_len);
}

void addIP(uint32_t address, void *context)
{
    if (address == EMPTY)
    {
        return;
    }

    uint32_t original_address = address;

    set_t *set = (set_t *)context;
    assert(set->table_used < set->table_len);
    size_t def_pos;
    size_t ins_pos = def_pos = hash(address) % set->table_len;
    for (; set->table[ins_pos] != EMPTY; ins_pos = (ins_pos + 1) % set->table_len)
    {
        if (set->table[ins_pos] == address)
        {
            return;
        }

        // if the displacement of the inserting element is further than the displacement of the current element
        // swap the two and continue
        if (displacementOf(set, ins_pos) < ins_pos - def_pos)
        {
            // record the displacement
            if (ins_pos - def_pos > set->max_dist)
            {
                set->max_dist = ins_pos - def_pos;
            }
            // swap the two
            uint32_t x = set->table[ins_pos];
            set->table[ins_pos] = address;
            address = x;
            // reset the counting and such
            ins_pos = def_pos = hash(address) % set->table_len;
        }
    }

    // update max displacement
    if (ins_pos - def_pos > set->max_dist)
    {
        set->max_dist = ins_pos - def_pos;
    }

    // drop final entry
    set->table[ins_pos] = address;
    set->table_used++;
}

void setAddAll(set_t *set, subnet_t addresses)
{
    iterateIPs(&addresses, set, addIP);
}

bool setContains(set_t *set, uint32_t address)
{
    if (address == EMPTY)
    {
        return true;
    }

    set->search_count++;
    size_t searched = 0;
    set->search_over_cache_line++;

    int def_pos, data_pos;
    for (data_pos = def_pos = hash(address) % set->table_len;
        set->table[data_pos] != EMPTY && (data_pos - def_pos) <= set->max_dist;
        data_pos = (data_pos + 1) % set->table_len)
    {
        if(searched % 16 == 0 && searched != 0)
        {
            set->search_over_cache_line++;
        }
        set->total_search_dist++;
        searched++;

        if (set->table[data_pos] == address)
        {
            return true;
        }
    }
    
    return false;
}

size_t setGetSize(set_t *set)
{
    return set->table_used;
}

void setPrintExtraStats(set_t * set)
{
    printf("Implementation = Robinhood Hashing\n");
    printf("load factor = %f\n", (float)set->table_used / (set->table_len));
    printf("Average Search Dist = %f\n", ((double) set->total_search_dist/set->search_count));
    printf("Searches over 16 elts = %lu\n", set->search_over_cache_line - set->search_count);
}
