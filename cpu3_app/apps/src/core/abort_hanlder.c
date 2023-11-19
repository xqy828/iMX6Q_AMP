#include "asm_defines.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

typedef struct {
    uint32_t ifar;
    uint32_t ifsr;
    uint32_t dfar;
    uint32_t dfsr;
    uint32_t cpsr;
    uint32_t pc;
    uint32_t sp;
    uint32_t lr;
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;
    uint32_t r12;
} arm_regs_t, *arm_regs_p;

int dump_regs(arm_regs_p regs)
{
    printf("r0 = 0x%08x    r8 =  0x%08x\n", regs->r0, regs->r8);
    printf("r1 = 0x%08x    r9 =  0x%08x\n", regs->r1, regs->r9);
    printf("r2 = 0x%08x    r10 = 0x%08x\n", regs->r2, regs->r10);
    printf("r3 = 0x%08x    r11 = 0x%08x\n", regs->r3, regs->r11);
    printf("r4 = 0x%08x    r12 = 0x%08x\n", regs->r4, regs->r12);
    printf("r5 = 0x%08x    sp =  0x%08x\n", regs->r5, regs->sp);
    printf("r6 = 0x%08x    lr =  0x%08x\n", regs->r6, regs->lr);
    printf("r7 = 0x%08x    pc =  0x%08x\n", regs->r7, regs->pc);
    return 0;
}







