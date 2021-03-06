#include "blacklist.h"

#include "lib/allocator.h"
#include "lib/common.h"

#include <signal.h>
#include <assert.h>

#define BLOCK_LEN 8
#define EMPTY 0
#define BUCKET_SIZE 14
#define PACK_SIZE ((BUCKET_SIZE + 3) / 4)
#define BLOOM_FILTER_SIZE (64 - (BUCKET_SIZE * 4 + PACK_SIZE))
#define MAX_LOOPS 16
#define TOP_HALF_MASK 0xfffffff8

typedef struct
{
    uint32_t data[BUCKET_SIZE];              // 14*4 = 56
    uint8_t packed_subnet_sizes[PACK_SIZE];  // 4 (Each subnet is 2 bits)
    uint8_t filter[BLOOM_FILTER_SIZE];       // 4
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
    static_assert(sizeof(table_bucket_t) == 64, "Bucket size should be cache line!");
    set_t *set = profiledCalloc(1, sizeof(set_t));
    set->table_len = ((float)(len * 0.1) / BUCKET_SIZE);
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

static inline uint8_t unpackSubnetSize(table_bucket_t *bucket, size_t i)
{
    uint8_t subnet_size = (bucket->packed_subnet_sizes[i / 4] >> ((i % 4) * 2)) & 0b00000011;
    return subnet_size;
}

static inline uint8_t setPackedSubnetSize(table_bucket_t *bucket, size_t i, uint8_t val)
{
    uint8_t insert_mask = val << ((i % 4) * 2);
    uint8_t clear_mask = ~(0b00000011 << ((i % 4) * 2));

    bucket->packed_subnet_sizes[i / 4] &= clear_mask;
    bucket->packed_subnet_sizes[i / 4] |= insert_mask;
}

void addIPs(subnet_t *address, void *context);

void expand(set_t *set)
{
    table_bucket_t *old_buckets = set->table;
    size_t old_table_len = set->table_len;

    set->table_len = (set->table_len * 1.05) + 1;
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
                subnet_t toAdd;
                toAdd.address = old_buckets[i].data[j];
                toAdd.sig_bits = 32 - unpackSubnetSize(old_buckets + i, j);
                addIPs(&toAdd, set);
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

static inline int checkBuckedSlot(table_bucket_t *bucket, size_t i, uint32_t address)
{
    if (bucket->data[i] == EMPTY)
    {
        return false;
    }

    uint8_t subnet_size = (bucket->packed_subnet_sizes[i / 4] >> ((i % 4) * 2)) & 0b00000011;
    if ((bucket->data[i] >> subnet_size) == (address >> subnet_size))
    {
        return true;
    }

    return 2;
}

void addIPs(subnet_t *subnet, void *context)
{
    set_t *set = (set_t *)context;

    // try to put in primary bucket
    uint32_t adr_hash1 = hash1(subnet->address & TOP_HALF_MASK);
    table_bucket_t *bucket = set->table + (adr_hash1 % set->table_len);

    for (size_t i = 0; i < BUCKET_SIZE; i++)
    {
        // TODO: make this work with overlapping subnets that start with the same address
        int res = checkBuckedSlot(bucket, i, subnet->address);
        if (res == true)
        {
            return;
        }

        if (bucket->data[i] == EMPTY)
        {
            bucket->data[i] = subnet->address;
            setPackedSubnetSize(bucket, i, 32 - subnet->sig_bits);
            set->insert_loops = 0;
            return;
        }
    }

    // set bloom filter bit
    uint32_t adr_hash2 = hash2(subnet->address & TOP_HALF_MASK);
    uint8_t segment = ((adr_hash2 / 8) % BLOOM_FILTER_SIZE);
    uint8_t mask = (1 << (adr_hash2 % 8));
    bucket->filter[segment] |= mask;

    // try to place in second bucket
    uint32_t offset = adr_hash2 % set->table_len;
    bucket = set->table + (offset);
    for (size_t i = 0; i < BUCKET_SIZE; i++)
    {
        int res = checkBuckedSlot(bucket, i, subnet->address);
        if (res == true)
        {
            return;
        }

        if (bucket->data[i] == EMPTY)
        {
            bucket->data[i] = subnet->address;
            setPackedSubnetSize(bucket, i, 32 - subnet->sig_bits);
            set->insert_loops = 0;

            return;
        }

        // check if here elt is in primary bucket (so we can displace it as needed)
        subnet_t old_subnet;
        old_subnet.address = bucket->data[i];
        old_subnet.sig_bits = 32 - unpackSubnetSize(bucket, i);
        uint32_t test_offset = hash1(old_subnet.address & TOP_HALF_MASK) % set->table_len;
        if (test_offset == offset)
        {
            bucket->data[i] = subnet->address;
            setPackedSubnetSize(bucket, i, 32 - subnet->sig_bits);
            set->insert_loops++;

            // If we are in a cycle, just rehash the whole thing
            if (set->insert_loops > MAX_LOOPS)
            {
                expand(set);
            }

            addIPs(&old_subnet, set);
            return;
        }
    }

    expand(set);
    addIPs(subnet, set);

    // printf("Buckets full, cuckoo chaining required\n");
    // exit(1);
}

void iterateSubnets(set_t *set, subnet_t *subnet, size_t subnet_size)
{
    uint32_t top_address = subnet->address >> subnet_size;

    uint8_t insig_count = (32 - subnet->sig_bits) - subnet_size;
    uint32_t sig_mask = ((1L << subnet->sig_bits) - 1) << insig_count;
    uint32_t insig_mask = (1 << insig_count);

    uint32_t base_adr = top_address & sig_mask;

    for (size_t i = 0; i < insig_mask; i++)
    {
        subnet_t sub_subnet;
        sub_subnet.address = ((top_address | i) << subnet_size);
        sub_subnet.sig_bits = 32 - subnet_size;

        addIPs(&sub_subnet, set);
    }
}

void setAddAll(set_t *set, subnet_t addresses)
{
    if (addresses.sig_bits > 28)
    {
        addIPs(&addresses, set);
    }
    else
    {
        iterateSubnets(set, &addresses, 3);
    }
}

bool setContains(set_t *set, uint32_t address)
{
    // check primary bucket
    uint32_t adr_hash1 = hash1(address & TOP_HALF_MASK);
    table_bucket_t *bucket = set->table + (adr_hash1 % set->table_len);
    uint8_t *subnet_size_seg = bucket->packed_subnet_sizes;
    size_t i = 0;
    while (i < BUCKET_SIZE)
    {
        if (bucket->data[i] == EMPTY)
        {
            return false;
        }

        uint8_t subnet_size = (*subnet_size_seg >> ((i % 4) * 2)) & 0b00000011;
        if (((address - bucket->data[i]) >> subnet_size) == 0)
        {
            return true;
        }

        i++;
        if (i % 4 == 0)
        {
            subnet_size_seg++;
        }
    }

    // check bloom filter bit
    uint32_t adr_hash2 = hash2(address & TOP_HALF_MASK);
    uint8_t segment = ((adr_hash2 / 8) % BLOOM_FILTER_SIZE);
    uint8_t mask = (1 << (adr_hash2 % 8));
    if ((bucket->filter[segment] & mask) == 0)
    {
        set->bloom_filter_misses++;
        return false;
    }

    set->second_bucket_queries++;
    // check secondary bucket
    bucket = set->table + (adr_hash2 % set->table_len);
    subnet_size_seg = bucket->packed_subnet_sizes;
    i = 0;
    while (i < BUCKET_SIZE)
    {
        if (bucket->data[i] == EMPTY)
        {
            return false;
        }

        uint8_t subnet_size = (*subnet_size_seg >> ((i % 4) * 2)) & 0b00000011;
        if (((address - bucket->data[i]) >> subnet_size) == 0)
        {
            set->second_bucket_hits++;
            return true;
        }

        i++;
        if (i % 4 == 0)
        {
            subnet_size_seg++;
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
                // uint8_t subnet_size = (set->table[i].packed_subnet_sizes[j / 4] >> ((j % 4) * 2)) & 0b00000011;
                size += (1 << unpackSubnetSize(&set->table[i], j));
            }
        }
    }
    return size;
}

void setPrintExtraStats(set_t *set)
{
    size_t size = 0;
    for (size_t i = 0; i < set->table_len; i++)
    {
        for (size_t j = 0; j < BUCKET_SIZE; j++)
        {
            if (set->table[i].data[j] != EMPTY)
            {
                size++;
            }
        }
    }

    printf("Implementation = Cuckoo Prefix\n");
    printf("Expansions = %lu, load factor = %f\n", set->expansions, (float)size / (set->table_len * BUCKET_SIZE));
    printf("Second Bucket Queries = %lu, Second bucket hits = %lu\n", set->second_bucket_queries, set->second_bucket_hits);
    printf("Bloom filter misses = %lu, Bucket Size = %u\n", set->bloom_filter_misses, BUCKET_SIZE);
}