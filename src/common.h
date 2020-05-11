#pragma once

#include <stdio.h>
#include <string.h>

#include "blacklist.h"

void iterateIPs(subnet_t *subnet, void * context, void (*f)(uint32_t, void *));

uint32_t addressAsInt(char * str);

char * addressAsStr(uint32_t address);