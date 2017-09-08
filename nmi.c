#include "nmi.h"
#include "utils.h"
#include<stdio.h>
#include<stdlib.h>

typedef uint32_t (*dispatcher)(VM *, uint8_t);



uint32_t * getReg(VM * vm, int n)
{
    uint32_t * addresses[] = { NULL, &vm->rga, &vm->rgb, &vm->rgc, &vm->rgd, &vm->rsp, &vm->rip, &vm->rfl };
    return addresses[n];
}

uint32_t get32(VM * vm)
{
    int a, b, c, d;
    a = vm->RAM[vm->rip++];
    b = vm->RAM[vm->rip++];
    c = vm->RAM[vm->rip++];
    d = vm->RAM[vm->rip++];

    return (a << 24) + (b << 16) + (c << 8) + d;
}

uint16_t get16(VM * vm)
{
    int a, b;

    a = vm->RAM[vm->rip++];
    b = vm->RAM[vm->rip++];

    return (uint16_t)((a << 8) + b);
}

uint8_t get8(VM * vm)
{
    return vm->RAM[vm->rip++];
}


uint32_t execute(VM * vm)
{
    uint32_t returnCode = 0;
    vm->running = 1;
    // get entry point
    vm->rip = 0;
    vm->rip = get16(vm);
    DEBUG("RIP is now %d\n", vm->rip);
    // while the stop flag is not set
    while(vm->running)
    {
        // execute a single instruction
        returnCode = execute_ni(vm);
#ifdef DEBUGGING
        printf("=== REGISTER DUMP ===\n");
        printf("RGA(%p): %d\n", &vm->rga, vm->rga);
        printf("RGB(%p): %d\n", &vm->rgb, vm->rgb);
        printf("RGC(%p): %d\n", &vm->rgc, vm->rgc);
        printf("RGD(%p): %d\n", &vm->rgd, vm->rgd);
        getchar();
#endif
    }
        
    return returnCode;
}

uint32_t sysint(VM * vm, uint8_t op)
{
    DEBUG("Interrupt #%d\n", vm->rga);
    // Compare RGA to a table of defined system interrupts
    switch(vm->rga) {
    case 0: // exit (exit code stored in RGB)
        vm->running = 0;
        return (uint32_t)vm->rgb;
    case 1: // print a single character (code stored in RGB)
        printf("%c", vm->RAM[vm->rgb]);
        return 0;
    case 2: { // print a null-terminated string (addr stored in RGB)
        uint8_t * c;
        for (c = &vm->RAM[vm->rgb]; *c; c++)
            printf("%c", *c);
        /*while((c = &vm->RAM[vm->rgb++]))
        {
            printf("%d\n", *c);
            printf("%c\n\n", *c);
        }*/
            
        return 0;
    }
        
    default: // die
        printf("Unknown system call: %d", vm->rga);
        exit(1);
    }
}

uint32_t move(VM * vm, uint8_t op)
{
    
    uint32_t * dest;
    uint32_t src;
    get_srcDest(vm, op, &src, &dest);
    DEBUG("Moving %d to %p\n", src, (void*)dest);
    *dest = src;
    return 0;
}

uint32_t getmask(uint8_t size)
{
    uint32_t mask = 0xFFFFFFFF;
    if(size == 1) mask = 0xFFFF;
    else if (size == 2) mask = 0xFF;
    else if (size == 3) mask = 0x0F;

    return mask;
}

void set(uint32_t * dest, uint32_t sum, uint32_t mask)
{
    *dest = (*dest & ~mask);
    *dest += (sum & mask);
}

uint32_t add(VM * vm, uint8_t op)
{
    uint32_t * dest;
    uint32_t src;
    uint8_t size = get_srcDest(vm, op, &src, &dest);
    uint32_t sum = *dest + src;
    
    uint32_t mask = getmask(size);
    int sign = (mask >> 1) + 1;

    SET_FLAG_TO(vm->rfl, CARRY, (sum > mask));
    SET_FLAG_TO(vm->rfl, OVERFLOW, ((!((src & sign) ^ (*dest & sign))) && ((src & sign) ^ (sum & sign))));
    SET_FLAG_TO(vm->rfl, PARITY, (bitcount(sum & mask)));

    set(dest, sum, mask);

    return 0;
    
}

uint32_t sub(VM * vm, uint8_t op)
{
    DEBUG("subtracting\n");
    uint32_t * dest;
    uint32_t src;

    uint8_t size = get_srcDest(vm, op, &src, &dest);
    DEBUG("Got dest = %x, *dest = %d, src = %d\n", (void*)dest, *dest, src);
    uint32_t sum = *dest - src;
    DEBUG("sum is %d\n", sum);
    uint32_t mask = getmask(size);
    int sign = (mask >> 1) + 1;

    SET_FLAG_TO(vm->rfl, BORROW, (sum > mask));
    SET_FLAG_TO(vm->rfl, OVERFLOW, ((!((src & sign) ^ (*dest & sign))) && ((src & sign) ^ (sum & sign))));
    SET_FLAG_TO(vm->rfl, PARITY, (bitcount(sum & mask)));

    set(dest, sum, mask);

    return 0;
}

uint32_t mul(VM * vm, uint8_t op)
{
    uint32_t * dest;
    uint32_t src;

    uint8_t size = get_srcDest(vm, op, &src, &dest);
    uint32_t sum = *dest * src;

    uint32_t mask = getmask(size);
    int sign = (mask >> 1) + 1;

    SET_FLAG_TO(vm->rfl, OVERFLOW, ((sum > mask) | ((!((src & sign) ^ (*dest & sign))) && ((src & sign) ^ (sum & sign)))));
    SET_FLAG_TO(vm->rfl, PARITY, (bitcount(sum & mask)));

    set(dest, sum, mask);

    return 0;
}

uint32_t and(VM * vm, uint8_t op)
{
    uint32_t * dest;
    uint32_t src;

    uint8_t size = get_srcDest(vm, op, &src, &dest);
    uint32_t sum = *dest & src;
    uint32_t mask = getmask(size);

    SET_FLAG_TO(vm->rfl, PARITY, (bitcount(sum & mask)));

    set(dest, sum, mask);

    return 0;
}

uint32_t or(VM * vm, uint8_t op)
{
    uint32_t * dest;
    uint32_t src;

    uint8_t size = get_srcDest(vm, op, &src, &dest);
    uint32_t sum = *dest | src;
    uint32_t mask = getmask(size);

    SET_FLAG_TO(vm->rfl, PARITY, (bitcount(sum & mask)));

    set(dest, sum, mask);

    return 0;
}

uint32_t xor(VM * vm, uint8_t op)
{
    uint32_t * dest;
    uint32_t src;

    uint8_t size = get_srcDest(vm, op, &src, &dest);
    uint32_t sum = *dest ^ src;
    uint32_t mask = getmask(size);

    SET_FLAG_TO(vm->rfl, PARITY, (bitcount(sum & mask)));

    set(dest, sum, mask);

    return 0;
}

uint32_t nand(VM * vm, uint8_t op)
{
    uint32_t * dest;
    uint32_t src;

    uint8_t size = get_srcDest(vm, op, &src, &dest);
    uint32_t sum = ~(*dest & src);
    uint32_t mask = getmask(size);

    SET_FLAG_TO(vm->rfl, PARITY, (bitcount(sum & mask)));

    set(dest, sum, mask);

    return 0;
}

uint32_t nor(VM * vm, uint8_t op)
{
    uint32_t * dest;
    uint32_t src;

    uint8_t size = get_srcDest(vm, op, &src, &dest);
    uint32_t sum = ~(*dest | src);
    uint32_t mask = getmask(size);

    SET_FLAG_TO(vm->rfl, PARITY, (bitcount(sum & mask)));

    set(dest, sum, mask);

    return 0;
}

uint32_t cmp(VM * vm, uint8_t op)
{
    uint32_t * dest;
    uint32_t src;

    uint8_t size = get_srcDest(vm, op, &src, &dest);

    uint32_t mask = getmask(size);

    uint32_t a, b;
    a = *dest & mask;
    b = src & mask;
    DEBUG("Comparing %d to %d\n", a, b);
    SET_FLAG_TO(vm->rfl, ZERO, (a == b));
    SET_FLAG_TO(vm->rfl, NEGATIVE, (a < b));

    return 0;
}



uint32_t jmp(VM * vm, uint8_t op)
{
    uint16_t dest = get16(vm);
    DEBUG("Jumping to 0x%x", dest);
    vm->rip = dest;
    return dest;
}

uint32_t jeq(VM * vm, uint8_t op)
{
    DEBUG("==\n");
    if (GET_FLAG(vm->rfl, ZERO))
        return jmp(vm, op);
    get16(vm);
    return 0;
}

uint32_t jne(VM * vm, uint8_t op)
{
    DEBUG("!=\n");
    if (!GET_FLAG(vm->rfl, ZERO))
        return jmp(vm, op);
    get16(vm);
    return 0;
}

uint32_t jlt(VM * vm, uint8_t op)
{
    DEBUG("<\n");
    if (GET_FLAG(vm->rfl, NEGATIVE))
        return jmp(vm, op);
    get16(vm);
    return 0;
}

uint32_t jgt(VM * vm, uint8_t op)
{
    DEBUG(">\n");
    if(!(GET_FLAG(vm->rfl, ZERO) | GET_FLAG(vm->rfl, NEGATIVE)))
        return jmp(vm, op);
    get16(vm);
    return 0;
}

uint32_t jle(VM * vm, uint8_t op)
{
    DEBUG("<=\n");
    if (GET_FLAG(vm->rfl, ZERO) | GET_FLAG(vm->rfl, NEGATIVE))
        return jmp(vm, op);
    get16(vm);
    return 0;
}

uint32_t jge(VM * vm, uint8_t op)
{
    DEBUG(">=\n");
    if (!GET_FLAG(vm->rfl, NEGATIVE))
        return jmp(vm, op);
    get16(vm);
    return 0;
}

uint32_t nop(VM * vm, uint8_t op)
{
    return 0;
}

void _push(VM * vm, uint32_t value)
{
    uint8_t a, b, c, d;
    a = value >> 24;
    b = value >> 16;
    c = value >> 8;
    d = value;

    vm->RAM[vm->rsp++] = (a & 0xFF);
    vm->RAM[vm->rsp++] = (b & 0xFF);
    vm->RAM[vm->rsp++] = (c & 0xFF);
    vm->RAM[vm->rsp++] = (d & 0xFF);
}

uint32_t _pop(VM * vm)
{
    uint32_t val = 0;
    val += vm->RAM[vm->rsp--];
    val += vm->RAM[vm->rsp--] << 8;
    val += vm->RAM[vm->rsp--] << 16;
    val += vm->RAM[vm->rsp--] << 24;

    return val;
}

uint32_t push(VM * vm, uint8_t op)
{
    uint32_t value;
    if ((op & 7) == 0) value = get32(vm);
    else value = (uint32_t)(*getReg(vm, (op & 7)));

    _push(vm, value);    

    return 0;
}



uint32_t pop(VM * vm, uint8_t op)
{
    uint32_t * dest = getReg(vm, (op & 7));
    

    *dest = _pop(vm);
    return 0;
}

uint32_t call(VM * vm, uint8_t op)
{
    
    uint16_t dest = get16(vm);
    DEBUG("Calling func @ 0x%x\n", dest);
    _push(vm, vm->rip);
    vm->rip = dest;

    return 0;
}

uint32_t ret(VM * vm, uint8_t op)
{

    uint32_t caller = _pop(vm);
    vm->rip = caller + 3; // offset of 3 to place RIP immediately after the "call" instruction

    return 0;
}


dispatcher funcs[] = {nop, push, pop, move, add, sub, mul, cmp, jmp, jeq, jne, jlt, jgt, jle, jge, call, ret, sysint, and, or, xor, nand, nor};

uint32_t execute_ni(VM * vm)
{
    DEBUG("RIP is %d (%x)\n", vm->rip, vm->rip);
    uint8_t op = get8(vm);
    DEBUG("Op is %d (%x); calling #%d\n", op, op, op>>3);
    dispatcher func = funcs[op >> 3];
    return func(vm, op);
}
