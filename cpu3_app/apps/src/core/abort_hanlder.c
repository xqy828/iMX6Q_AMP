#include "asm_defines.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include "public.h"
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
struct pt_regs g_arm_regs_t;
struct user_vfp g_arm_vfp_t;

extern int do_coredump(void);
extern void dump_stack_isr(struct pt_regs *regs);

void vfp_read_d02d31(struct user_vfp *ctx)
{
    asm __volatile__ (
        "vstm %0, {d0 - d15}\n"
        :
        : "r"(&ctx->fpregs[0])
        : "memory"
    );
    asm __volatile__ (
        "vstm %0, {d16 - d31}\n"
        :
        : "r"(&ctx->fpregs[16])
        : "memory"
    );
    asm __volatile__ ("vmrs %0, fpscr" : "=r"(ctx->fpscr));
}

int dump_regs(int abortType, arm_regs_p regs)
{
    unsigned long long val = 0;
    unsigned long long val_1 = 0;
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
    vfp_read_d02d31(&g_arm_vfp_t);
    printf("fpscr: 0x%016x\n", g_arm_vfp_t.fpscr);

    val = g_arm_vfp_t.fpregs[0];
    val_1 = g_arm_vfp_t.fpregs[8];
    printf("D0 = 0x%08lx%08lx     D8 =  0x%08lx%08lx\n",  
        (unsigned long)(val >> 32), (unsigned long)(val),(unsigned long)(val_1 >> 32), (unsigned long)(val_1));
    val = g_arm_vfp_t.fpregs[1];
    val_1 = g_arm_vfp_t.fpregs[9];
    printf("D1 = 0x%08lx%08lx     D9 =  0x%08lx%08lx\n",  
        (unsigned long)(val >> 32), (unsigned long)(val),(unsigned long)(val_1 >> 32), (unsigned long)(val_1));
    val = g_arm_vfp_t.fpregs[2];
    val_1 = g_arm_vfp_t.fpregs[10];
    printf("D2 = 0x%08lx%08lx     D10 =  0x%08lx%08lx\n",  
        (unsigned long)(val >> 32), (unsigned long)(val),(unsigned long)(val_1 >> 32), (unsigned long)(val_1));
    val = g_arm_vfp_t.fpregs[3];
    val_1 = g_arm_vfp_t.fpregs[11];
    printf("D3 = 0x%08lx%08lx     D11 =  0x%08lx%08lx\n",  
        (unsigned long)(val >> 32), (unsigned long)(val),(unsigned long)(val_1 >> 32), (unsigned long)(val_1));
    val = g_arm_vfp_t.fpregs[4];
    val_1 = g_arm_vfp_t.fpregs[12];
    printf("D4 = 0x%08lx%08lx     D12 =  0x%08lx%08lx\n",  
        (unsigned long)(val >> 32), (unsigned long)(val),(unsigned long)(val_1 >> 32), (unsigned long)(val_1));
    val = g_arm_vfp_t.fpregs[5];
    val_1 = g_arm_vfp_t.fpregs[13];
    printf("D5 = 0x%08lx%08lx     D13 =  0x%08lx%08lx\n",  
        (unsigned long)(val >> 32), (unsigned long)(val),(unsigned long)(val_1 >> 32), (unsigned long)(val_1));
    val = g_arm_vfp_t.fpregs[6];
    val_1 = g_arm_vfp_t.fpregs[14];
    printf("D6 = 0x%08lx%08lx     D14 =  0x%08lx%08lx\n",  
        (unsigned long)(val >> 32), (unsigned long)(val),(unsigned long)(val_1 >> 32), (unsigned long)(val_1));
    val = g_arm_vfp_t.fpregs[7];
    val_1 = g_arm_vfp_t.fpregs[15];
    printf("D7 = 0x%08lx%08lx     D15 =  0x%08lx%08lx\n",  
        (unsigned long)(val >> 32), (unsigned long)(val),(unsigned long)(val_1 >> 32), (unsigned long)(val_1));
    printf("====================================================\n");
    val = g_arm_vfp_t.fpregs[16];
    val_1 = g_arm_vfp_t.fpregs[24];
    printf("D16 = 0x%08lx%08lx     D24 =  0x%08lx%08lx\n",  
        (unsigned long)(val >> 32), (unsigned long)(val),(unsigned long)(val_1 >> 32), (unsigned long)(val_1));
    val = g_arm_vfp_t.fpregs[17];
    val_1 = g_arm_vfp_t.fpregs[25];
    printf("D17 = 0x%08lx%08lx     D25=  0x%08lx%08lx\n",  
        (unsigned long)(val >> 32), (unsigned long)(val),(unsigned long)(val_1 >> 32), (unsigned long)(val_1));
    val = g_arm_vfp_t.fpregs[18];
    val_1 = g_arm_vfp_t.fpregs[26];
    printf("D18 = 0x%08lx%08lx     D26 =  0x%08lx%08lx\n",  
        (unsigned long)(val >> 32), (unsigned long)(val),(unsigned long)(val_1 >> 32), (unsigned long)(val_1));
    val = g_arm_vfp_t.fpregs[19];
    val_1 = g_arm_vfp_t.fpregs[27];
    printf("D19 = 0x%08lx%08lx     D27 =  0x%08lx%08lx\n",  
        (unsigned long)(val >> 32), (unsigned long)(val),(unsigned long)(val_1 >> 32), (unsigned long)(val_1));
    val = g_arm_vfp_t.fpregs[20];
    val_1 = g_arm_vfp_t.fpregs[28];
    printf("D20 = 0x%08lx%08lx     D28 =  0x%08lx%08lx\n",  
        (unsigned long)(val >> 32), (unsigned long)(val),(unsigned long)(val_1 >> 32), (unsigned long)(val_1));
    val = g_arm_vfp_t.fpregs[21];
    val_1 = g_arm_vfp_t.fpregs[29];
    printf("D21 = 0x%08lx%08lx     D29 =  0x%08lx%08lx\n",  
        (unsigned long)(val >> 32), (unsigned long)(val),(unsigned long)(val_1 >> 32), (unsigned long)(val_1));
    val = g_arm_vfp_t.fpregs[22];
    val_1 = g_arm_vfp_t.fpregs[30];
    printf("D22 = 0x%08lx%08lx     D30 =  0x%08lx%08lx\n",  
        (unsigned long)(val >> 32), (unsigned long)(val),(unsigned long)(val_1 >> 32), (unsigned long)(val_1));
    val = g_arm_vfp_t.fpregs[23];
    val_1 = g_arm_vfp_t.fpregs[31];
    printf("D23 = 0x%08lx%08lx     D31 =  0x%08lx%08lx\n",  
        (unsigned long)(val >> 32), (unsigned long)(val),(unsigned long)(val_1 >> 32), (unsigned long)(val_1));

    g_arm_regs_t.ARM_r0 = regs->r0;
    g_arm_regs_t.ARM_r1 = regs->r1;
    g_arm_regs_t.ARM_r2 = regs->r2;
    g_arm_regs_t.ARM_r3 = regs->r3;
    g_arm_regs_t.ARM_r4 = regs->r4;
    g_arm_regs_t.ARM_r5 = regs->r5;
    g_arm_regs_t.ARM_r6 = regs->r6;
    g_arm_regs_t.ARM_r7 = regs->r7;
    g_arm_regs_t.ARM_r8 = regs->r8;
    g_arm_regs_t.ARM_r9 = regs->r9;
    g_arm_regs_t.ARM_r10 = regs->r10;
    g_arm_regs_t.ARM_fp = regs->r11;
    g_arm_regs_t.ARM_ip = regs->r12;
    g_arm_regs_t.ARM_sp = regs->sp;
    g_arm_regs_t.ARM_lr = regs->lr;
    g_arm_regs_t.ARM_pc = regs->pc;
    g_arm_regs_t.ARM_cpsr = regs->cpsr;
    dump_stack_isr(&g_arm_regs_t);
    do_coredump();
    return 0;
}

unsigned int get_save_sp(void)
{
    return  g_arm_regs_t.ARM_sp;
}

unsigned int get_save_regs(struct pt_regs **regs)
{
    *regs = &g_arm_regs_t;
    return 0;
}

unsigned int get_save_vfp_regs(struct user_vfp **regs)
{
    *regs = &g_arm_vfp_t;
    return 0;
}