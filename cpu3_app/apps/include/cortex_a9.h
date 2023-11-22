
#if !defined(__CORTEX_A9_H__)
#define __CORTEX_A9_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define _arm_mrc(coproc, opcode1, Rt, CRn, CRm, opcode2)	\
    asm volatile ("mrc p" #coproc ", " #opcode1 ", %[output], c" #CRn ", c" #CRm ", " #opcode2 "\n" : [output] "=r" (Rt))
    
#define _arm_mcr(coproc, opcode1, Rt, CRn, CRm, opcode2)	\
    asm volatile ("mcr p" #coproc ", " #opcode1 ", %[input], c" #CRn ", c" #CRm ", " #opcode2 "\n" :: [input] "r" (Rt))

void Enable_SIMD_VFP(void);
int get_cpuid(void);
unsigned int read_cpsr_reg(void);


#endif
