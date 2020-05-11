#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#include "vector.h"
#include "fast_rand.h"
#include "common.h"

#include "blacklist.h"

#define HIT_RATE 10
#define TEST_COUNT 100

subnet_t parse_subnet(char *str)
{
    subnet_t rtn;
    rtn.address = 0;
    rtn.sig_bits = 32;

    char *curr = str;
    char *buf_start = str;
    // take in 4 bytes
    for (size_t i = 0; i < 4; i++)
    {
        while (true)
        {
            if ('0' <= *curr && *curr <= '9')
            {
                curr++;
                continue;
            }
            char tmp = *curr;
            *curr = 0;
            rtn.address += atoi(buf_start) << ((3 - i) * 8);
            *curr = tmp;
            curr++;
            buf_start = curr;
            break;
        }
    }

    // take in significant bit mask
    if (*curr != 0)
    {
        rtn.sig_bits = atoi(curr);
    }

    return rtn;
}

void strip(char *str)
{
    char *last = str + strlen(str) - 1;
    if (*last == '\n')
    {
        *last = 0;
    }
}

vector_t *parseFile(char *infile)
{
    vector_t *subnet_vector = newVector(sizeof(subnet_t));

    FILE *fp = fopen(infile, "r");
    assert(fp != NULL);

    ssize_t read = 0;
    size_t len = 0;
    char *line = NULL;
    while ((read = getline(&line, &len, fp)) != -1)
    {
        if (read == 1)
        {
            continue;
        }
        strip(line);
        if (line[0] == '#')
        {
            continue;
        }
        subnet_t subnet = parse_subnet(line);
        vectorAdd(subnet_vector, &subnet);
    }

    fclose(fp);
    free(line);

    return subnet_vector;
}

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

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        printf("Usage: %s <ip file>\n", argv[0]);
        return 1;
    }

    vector_t *subnet_vector = parseFile(argv[1]);
    size_t totalIPs = countIPs(subnet_vector);
    set_t *blocklist = newSet(totalIPs);

    for (size_t i = 0; i < subnet_vector->used; i++)
    {
        subnet_t *subnet = (subnet_t *)vectorGet(subnet_vector, i);
        setAddAll(blocklist, *subnet);
    }

    for (size_t i = 0; i < TEST_COUNT; i++)
    {
        // printf("-");
        uint32_t address;
        bool hit = i % HIT_RATE == 0;
        if(hit)
        {
            subnet_t * subnet = (subnet_t *)
                vectorGet(subnet_vector, i % subnet_vector->used);
            address = subnet->address;
            uint32_t networkIp = fastRand();
            size_t insig_count = (32 - subnet->sig_bits);
            size_t mask = (1 << (insig_count)) - 1;
            address |= networkIp & mask;
        }
        else
        {
            address = fastRand();
        }

        if(hit != setContains(blocklist, address))
        {
            char * adr = addressAsStr(address);
            printf("[FAILED] address = %s, hit = %i\n", adr, hit);
            free(adr);
            return 1;
        }
    }

    printf("[PASSED] TEST_COUNT = %u, HIT_RATE = %u\n", TEST_COUNT, HIT_RATE);

    deleteVector(subnet_vector);
}