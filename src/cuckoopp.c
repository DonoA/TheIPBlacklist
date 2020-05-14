#include "blacklist.h"

#include "lib/allocator.h"
#include "lib/common.h"

#define BLOCK_LEN 8
#define EMPTY 0
#define BUCKET_SIZE 14
#define MAX_LOOPS 16

typedef struct
{
    uint32_t data[BUCKET_SIZE];
    uint64_t filter;
} table_bucket_t;

struct set_struct
{
    table_bucket_t *table;
    size_t table_len;
    size_t insert_loops;

    size_t expansions;
    size_t bloom_filter_misses;
    size_t second_bucket_hits;
    size_t second_bucket_queries;
};

set_t *newSet(size_t len, float load_factor)
{
    set_t *set = profiledCalloc(1, sizeof(set_t));
    set->table_len = ((float)(len * load_factor) / BUCKET_SIZE);
    set->table = profiledCalloc(set->table_len, sizeof(table_bucket_t));
    set->insert_loops = 0;
    set->expansions = 0;
    return set;
}

void deleteSet(set_t *set)
{
    profiledFree(set->table);
    profiledFree(set);
}

void addIP(uint32_t address, void *context);

void expand(set_t *set)
{
    table_bucket_t *old_buckets = set->table;
    size_t old_table_len = set->table_len;

    set->table_len = (set->table_len * EXPAND_FACTOR_MOD) + 1;
    set->insert_loops = 0;
    set->table = profiledCalloc(set->table_len, sizeof(table_bucket_t));
    set->expansions++;

    // Add all ips from old set
    for (size_t i = 0; i < old_table_len; i++)
    {
        for (size_t j = 0; j < BUCKET_SIZE; j++)
        {
            if (old_buckets[i].data[j] != EMPTY)
            {
                addIP(old_buckets[i].data[j], set);
            }
        }
    }

    // free old bucket
    profiledFree(old_buckets);
}

// Seems to be pretty good
// https://github.com/skeeto/hash-prospector
static inline uint32_t hash1(uint32_t x)
{
    x ^= x >> 16;
    x *= UINT32_C(0x7feb352d);
    x ^= x >> 15;
    x *= UINT32_C(0x846ca68b);
    x ^= x >> 16;
    return x & 0xffffffff;
}

// Second hash functions also from the prospector
static inline uint32_t hash2(uint32_t x)
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
    set_t *set = (set_t *)context;

    // try to put in primary bucket
    uint32_t adr_hash1 = hash1(address);
    table_bucket_t *bucket = set->table + (adr_hash1 % set->table_len);
    for (size_t i = 0; i < BUCKET_SIZE; i++)
    {
        if (bucket->data[i] == address)
        {
            return;
        }

        if (bucket->data[i] == EMPTY)
        {
            bucket->data[i] = address;
            set->insert_loops = 0;
            return;
        }
    }

    // set bloom filter bit
    uint32_t adr_hash2 = hash2(address);
    uint64_t mask = (1 << (adr_hash2 % 64));
    bucket->filter |= mask;

    // try to place in second bucket
    uint32_t offset = adr_hash2 % set->table_len;
    bucket = set->table + (offset);
    for (size_t i = 0; i < BUCKET_SIZE; i++)
    {
        if (bucket->data[i] == address)
        {
            return;
        }

        if (bucket->data[i] == EMPTY)
        {
            bucket->data[i] = address;
            set->insert_loops = 0;

            return;
        }

        // check if here elt is in primary bucket (so we can displace it as needed)
        uint32_t old_ard = bucket->data[i];
        uint32_t test_offset = hash1(old_ard) % set->table_len;
        if (test_offset == offset)
        {
            bucket->data[i] = address;
            set->insert_loops++;

            // If we are in a cycle, just rehash the whole thing
            if (set->insert_loops > MAX_LOOPS)
            {
                expand(set);
            }

            addIP(old_ard, set);
            return;
        }
    }

    expand(set);
    addIP(address, set);
}

void setAddAll(set_t *set, subnet_t addresses)
{
    iterateIPs(&addresses, set, addIP);
}

bool setContains(set_t *set, uint32_t address)
{
    // check primary bucket
    uint32_t adr_hash1 = hash1(address);
    table_bucket_t *bucket = set->table + (adr_hash1 % set->table_len);
    for (size_t i = 0; i < BUCKET_SIZE; i++)
    {
        if (bucket->data[i] == address)
        {
            return true;
        }

        if (bucket->data[i] == EMPTY)
        {
            return false;
        }
    }

    // check bloom filter bit
    uint32_t adr_hash2 = hash2(address);
    uint64_t mask = (1 << (adr_hash2 % 64));
    if ((bucket->filter & mask) == 0)
    {
        set->bloom_filter_misses++;
        return false;
    }

    set->second_bucket_queries++;
    // check secondary bucket
    bucket = set->table + (adr_hash2 % set->table_len);
    for (size_t i = 0; i < BUCKET_SIZE; i++)
    {
        if (bucket->data[i] == address)
        {
            set->second_bucket_hits++;
            return true;
        }

        if (bucket->data[i] == EMPTY)
        {
            return false;
        }
    }

    return false;
}

size_t setGetSize(set_t *set)
{
    size_t size = 0;
    for (size_t i = 0; i < set->table_len; i++)
    {
        for (size_t j = 0; j < BUCKET_SIZE; j++)
        {
            if (set->table[i].data[j] != EMPTY)
            {
                // putchar('#');
                size++;
            }
            else
            {
                // putchar('-');
            }
        }
    }
    // putchar('\n');
    return size;
}

void setPrintExtraStats(set_t *set)
{
    size_t size = setGetSize(set);

    printf("Expansions = %lu, load factor = %f\n", set->expansions, (float)size / (set->table_len * BUCKET_SIZE));
    printf("Second Bucket Queries = %lu, Second bucket hits = %lu\n", set->second_bucket_queries, set->second_bucket_hits);
    printf("Bloom filter misses = %lu\n", set->bloom_filter_misses);
}