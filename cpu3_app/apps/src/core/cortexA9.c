#include "public.h"
#include "cortex_a9.h"

unsigned int read_cpsr_reg(void)
{
    unsigned int cpsr_reg = 0;
    asm volatile(
        "mrs %0, cpsr\n"
        : "=r" (cpsr_reg)
        :
        :
                );
	return cpsr_reg;
}




