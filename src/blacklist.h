#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define LOAD_FACTOR_MOD 0.1
#define EXPAND_FACTOR_MOD 1.1

typedef struct {
    uint32_t address;
    uint8_t sig_bits;
} subnet_t;

typedef struct set_struct set_t;

set_t * newSet(size_t length, float load_factor);
void deleteSet(set_t * set);

void setAddAll(set_t * set, subnet_t addresses);
bool setContains(set_t * set, uint32_t address);
size_t setGetSize(set_t * set);
void setPrintExtraStats(set_t * set);