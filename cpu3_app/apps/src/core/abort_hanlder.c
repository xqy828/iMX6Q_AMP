#include "asm_defines.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

#define BM_DFSR_WNR (1 << 11)   //!< Write not Read bit. 0=read, 1=write.
#define BM_DFSR_FS4 (0x400)      //!< Fault status bit 4..
#define BP_DFSR_FS4 (10)        //!< Bit position for FS[4].
#define BM_DFSR_FS (0xf)      //!< Fault status bits [3:0].

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

enum {
    kDataAbortType = 0,
    kPrefetchAbortType = 1
};

int dump_regs(int abortType, arm_regs_p regs)
{
    printf("\nOops, %s abort occurred!\n\n", (abortType == kDataAbortType) ? "data" : "prefetch");
    printf("Registers at point of exception:\n");

    // Get the name of the mode.
    char * cpsrModeName = NULL;
    switch (regs->cpsr & CPSR_MODE)
    {
        case MODE_USR:
            cpsrModeName = "User";
            break;
        case MODE_FIQ:
            cpsrModeName = "FIQ";
            break;
        case MODE_IRQ:
            cpsrModeName = "IRQ";
            break;
        case MODE_SVC:
            cpsrModeName = "Supervisor";
            break;
        case MODE_ABT:
            cpsrModeName = "Abort";
            break;
        case MODE_UND:
            cpsrModeName = "Undefined";
            break;
        case MODE_SYS:
            cpsrModeName = "System";
            break;
    }

    // nzcvqeaift
    printf("cpsr = %c%c%c%c%c%c%c%c%c%c %s (0x%08x)\n",
        ((regs->cpsr & CPSR_N) ? 'N' : 'n'),
        ((regs->cpsr & CPSR_Z) ? 'Z' : 'z'),
        ((regs->cpsr & CPSR_C) ? 'C' : 'c'),
        ((regs->cpsr & CPSR_V) ? 'V' : 'v'),
        ((regs->cpsr & CPSR_Q) ? 'Q' : 'q'),
        ((regs->cpsr & CPSR_E) ? 'E' : 'e'),
        ((regs->cpsr & CPSR_A) ? 'A' : 'a'),
        ((regs->cpsr & CPSR_I) ? 'I' : 'i'),
        ((regs->cpsr & CPSR_F) ? 'F' : 'f'),
        ((regs->cpsr & CPSR_T) ? 'T' : 't'),
        cpsrModeName,
        regs->cpsr);

    printf("r0 = 0x%08x    r8 =  0x%08x\n", regs->r0, regs->r8);
    printf("r1 = 0x%08x    r9 =  0x%08x\n", regs->r1, regs->r9);
    printf("r2 = 0x%08x    r10 = 0x%08x\n", regs->r2, regs->r10);
    printf("r3 = 0x%08x    r11 = 0x%08x\n", regs->r3, regs->r11);
    printf("r4 = 0x%08x    r12 = 0x%08x\n", regs->r4, regs->r12);
    printf("r5 = 0x%08x    sp =  0x%08x\n", regs->r5, regs->sp);
    printf("r6 = 0x%08x    lr =  0x%08x\n", regs->r6, regs->lr);
    printf("r7 = 0x%08x    pc =  0x%08x\n", regs->r7, regs->pc);

    uint32_t fsr;
    if (abortType == kDataAbortType)
    {
        printf("dfsr = 0x%08x\n", regs->dfsr);
        printf("dfar = 0x%08x\n", regs->dfar);
        fsr = regs->dfsr;
        printf("\nAccess type: %s\n", (regs->dfsr & BM_DFSR_WNR) ? "write" : "read");
    }
    else
    {
        printf("ifsr = 0x%08x\n", regs->ifsr);
        printf("ifar = 0x%08x\n", regs->ifar);
        fsr = regs->ifsr;
    }

    uint32_t faultStatus = ((fsr & BM_DFSR_FS4) >> BP_DFSR_FS4) | (fsr & BM_DFSR_FS);
    printf("Fault status: 0x%x\n", faultStatus);

    return 0;
}







