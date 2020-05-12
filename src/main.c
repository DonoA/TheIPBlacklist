#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <time.h>

#include "lib/vector.h"
#include "lib/fast_rand.h"
#include "lib/common.h"
#include "lib/allocator.h"
#include "lib/subnet_loader.h"

#include "blacklist.h"

#define HIT_RATE 2
// #define TEST_COUNT 50 * 1000 * 1000
#define TEST_COUNT 50
// #define TEST_SIZE 10 * 1000
#define TEST_SIZE 10
#define DEBUG true

size_t countIPs(vector_t *subnet_vector)
{
    size_t total = 0;
    for (size_t i = 0; i < subnet_vector->used; i++)
    {
        subnet_t *subnet = (subnet_t *)vectorGet(subnet_vector, i);
        total += 1 << (32 - subnet->sig_bits);
    }

    return total;
}

void runTest()
{
    set_t *blocklist = newSet(TEST_SIZE);

    size_t r = 1;
    for (size_t i = 0; i < TEST_SIZE; i++)
    {
        r = (r * 12345);
        uint32_t address = (r | 0x11111111) & 0xff00ff00;

        subnet_t subnet;
        subnet.address = address;
        subnet.sig_bits = 32;

        setAddAll(blocklist, subnet);
    }

    for (size_t i = 0; i < TEST_COUNT; i++)
    {
        if ((i * 2) % TEST_SIZE == 0)
        {
            r = 1;
        }

        uint32_t address;
        bool hit = i % HIT_RATE == 0;

        if (hit)
        {
            r = (r * 12345);
            address = (r | 0x11111111) & 0xff00ff00;
        }
        else
        {
            address = (r | 0x11111111) & 0x00ff00ff;
        }

        bool blocked = setContains(blocklist, address);

        if (blocked != hit)
        {
            char *adr = addressAsStr(address);
            printf("[FAILED] address = %s, hit = %i\n", adr, hit);
            assert(false);
            free(adr);
            exit(EXIT_FAILURE);
        }
    }

    printf("[PASSED] TEST_COUNT = %u, HIT_RATE = %u, IPs = %u\n", TEST_COUNT, HIT_RATE, TEST_SIZE);
}

void runProfiling(size_t totalIPs, vector_t *subnet_vector)
{
    clock_t start, stop;
    beingMemoryProfiling();

    start = clock();
    set_t *blocklist = newSet(totalIPs);
    for (size_t i = 0; i < subnet_vector->used; i++)
    {
        subnet_t *subnet = (subnet_t *)vectorGet(subnet_vector, i);
        setAddAll(blocklist, *subnet);
    }
    stop = clock();
    double setup_time = (double)(stop - start) / CLOCKS_PER_SEC;

    start = clock();
    for (size_t i = 0; i < TEST_COUNT; i++)
    {
        uint32_t address;
        bool hit = i % HIT_RATE == 0;
        if (hit)
        {
            subnet_t *subnet = (subnet_t *)
                vectorGet(subnet_vector, i % subnet_vector->used);
            address = subnet->address;
            uint8_t insig_count = (32 - subnet->sig_bits);
            size_t sig_mask = ((1L << subnet->sig_bits) - 1) << insig_count;
            size_t insig_mask = (1 << insig_count) - 1;

            uint32_t base_adr = subnet->address & sig_mask;

            uint32_t networkIp = fastRand();
            address |= networkIp & insig_mask;
        }
        else
        {
            address = fastRand();
        }

        bool contains = setContains(blocklist, address);
    }
    stop = clock();
    double test_time = (double)(stop - start) / CLOCKS_PER_SEC;

    printf("[PASSED] TEST_COUNT = %u, HIT_RATE = %u, IPs = %lu\n", TEST_COUNT, HIT_RATE, totalIPs);
    size_t allocated, freed;
    endProfiling(&allocated, &freed);
    printf("[PASSED] allocated (Bytes) = %lu, freed (Bytes) = %lu, set size = %lu\n", allocated, freed, setGetSize(blocklist));
    printf("[PASSED] setup time = %f (s), test_time = %f (s)\n", setup_time, test_time);

    deleteSet(blocklist);
}

int main(int argc, char *argv[])
{
    if (DEBUG)
    {
        runTest();
        return EXIT_SUCCESS;
    }

    if (argc != 2)
    {
        printf("Usage: %s <ip file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    vector_t *subnet_vector = newVector(sizeof(subnet_t));
    parseFile(subnet_vector, argv[1]);
    size_t totalIPs = countIPs(subnet_vector);

    runProfiling(totalIPs, subnet_vector);

    deleteVector(subnet_vector);

    return EXIT_SUCCESS;
}