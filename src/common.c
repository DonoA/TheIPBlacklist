#include "common.h"

void iterateIPs(subnet_t *subnet, void * context, void (*f)(uint32_t, void *))
{
    uint8_t insig_count = (32 - subnet->sig_bits);
    size_t sig_mask = ((1L << subnet->sig_bits) - 1) << insig_count;
    size_t insig_mask = (1 << insig_count);

    uint32_t base_adr = subnet->address & sig_mask;

    for (size_t i = 0; i < insig_mask; i++)
    {
        f(base_adr | i, context);
    }
}

uint32_t addressAsInt(char * str)
{
    uint32_t address = 0;

    char * buf_clone = (char *) calloc(strlen(str) + 1, sizeof(char));
    strcpy(buf_clone, str);

    char *curr = buf_clone;
    char *buf_start = buf_clone;
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
            address += atoi(buf_start) << ((3 - i) * 8);
            *curr = tmp;
            curr++;
            buf_start = curr;
            break;
        }
    }

    free(buf_clone);

    return address;
}

char * addressAsStr(uint32_t address)
{
    char * buff = (char *) calloc(16, sizeof(char));
    sprintf(buff, "%u.%u.%u.%u", 
        (address & 0xff000000) >> 24,
        (address & 0x00ff0000) >> 16,
        (address & 0x0000ff00) >> 8,
        (address & 0x000000ff));
    return buff;
}