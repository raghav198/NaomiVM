#ifndef NMI
#define NMI

#include<stdint.h>
#define SET_FLAG(reg, flag) do {reg = reg | flag;} while(0)
#define SET_FLAG_TO(reg, flag, val) do {reg = val ? (reg | flag) : (reg & ~flag);} while(0)
#define GET_FLAG(reg, flag) (reg & flag)


#define CARRY 0x80
#define OVERFLOW 0x40
#define BORROW 0x20
#define PARITY 0x10
#define NEGATIVE 0x08
#define ZERO 0x04

typedef struct VM_s {
    // general purpose registers
    uint32_t rga, rgb, rgc, rgd;
    // stack pointer, instruction pointer, and flags
    uint32_t rsp, rip, rfl;
    // memory (data, code, and stack)
    uint8_t RAM[65535];

    int running;
} VM;

enum {
    NOP,
    PUSH,
    POP,
    MOV,
    ADD,
    SUB,
    MUL,
    CMP,
    JMP,
    JEQ,
    JNE,
    JLT,
    JGT,
    JLE,
    JGE,
    CALL,
    RET,
    SYSINT,
    AND,
    OR,
    NOT,
    XOR,
    NAND,
    NOR
};


uint32_t execute(VM *);
uint32_t execute_ni(VM *);
uint32_t get32(VM *);
uint16_t get16(VM *);
uint8_t get8(VM *);
uint32_t * getReg(VM *, int);

#endif
