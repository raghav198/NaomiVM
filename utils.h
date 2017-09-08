#ifndef UTILS
#define UTILS

#ifdef DEBUGGING
#define DEBUG printf
#else
#define DEBUG //
#endif

#include<stdint.h>
#include "nmi.h"
void die(char *);
int bitcount(uint32_t);
uint8_t get_srcDest(VM *, uint8_t, uint32_t *, uint32_t **);

#endif
