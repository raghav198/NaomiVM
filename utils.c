#include "utils.h"

#include<stdio.h>
#include<stdlib.h>


void die(char * msg)
{
    printf("ERROR: %s", msg);
    exit(1);
}

int bitcount(uint32_t val)
{
    int bits = 0;
    while (val)
    {
        bits += val & 1;
        val = val >> 1;

    }
    return bits;
}

uint8_t get_srcDest(VM * vm, uint8_t op, uint32_t * src, uint32_t ** dest)
{
    uint8_t opt = get8(vm);
    if ((op & 7) == 0)
    {
        uint16_t addr = get16(vm);
        DEBUG("Memory address! (@%d)\n", addr);
        *dest = (uint32_t *)(vm->RAM + addr);
    }
    else
    {
        DEBUG("Register! (#%d)\n",  op & 7);
        *dest = getReg(vm, (op & 7));
    }
    uint8_t size = opt & 3;
    if (opt & 0x80)
    {
        if (size == 0) *src = get32(vm);
        else if (size == 1) *src = get16(vm);
        else if (size == 2) *src = get8(vm);
        else if (size == 3) *src = get8(vm) & 0x0F;
    }
    else
    {
        if ((opt & 0x7F) >> 4 == 0)
            *src = vm->RAM[get16(vm)];
        else
            *src = *getReg(vm, ((opt & 0x7F) >> 4));
    }
    DEBUG("Source = %d, Dest = %p\n", *src, (void*)(*dest));
    return size;
}
