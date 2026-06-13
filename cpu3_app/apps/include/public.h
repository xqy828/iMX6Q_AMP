#ifndef PUBLIC_H_ 
#define PUBLIC_H_
#include <stdio.h>
#include <string.h>
#include "cortex_a9.h"

#define UNUSED_PARA(para)   ((void)(para)) 

#ifdef DEBUG
#define _DEBUG  1
#else
#define _DEBUG  0
#endif

#define disp(format, ...)  \
do{\
    unsigned int cpuid = 0xf;\
    cpuid = get_cpuid();\
    printf("[\033[0;31m-CPU%d-\033[0m]:%s-(%05d)]"format,cpuid,__FUNCTION__,__LINE__,##__VA_ARGS__);\
}while(0)

#define debug_cond(cond, fmt, args...)		\
do{						\
    if (cond)				\
    disp(fmt, ##args);	\
}while(0)
 
#define debug(fmt, args...)          \
    debug_cond(_DEBUG, fmt, ##args)


#define U32_MAX         (4294967295u)                                  
#define U64_MAX         (18446744073709551615u)

struct pt_regs 
{
    unsigned long uregs[18];
};

#define ARM_cpsr	uregs[16]
#define ARM_pc		uregs[15]
#define ARM_lr		uregs[14]
#define ARM_sp		uregs[13]
#define ARM_ip		uregs[12]
#define ARM_fp		uregs[11]
#define ARM_r10		uregs[10]
#define ARM_r9		uregs[9]
#define ARM_r8		uregs[8]
#define ARM_r7		uregs[7]
#define ARM_r6		uregs[6]
#define ARM_r5		uregs[5]
#define ARM_r4		uregs[4]
#define ARM_r3		uregs[3]
#define ARM_r2		uregs[2]
#define ARM_r1		uregs[1]
#define ARM_r0		uregs[0]
#define ARM_ORIG_r0	uregs[17]

struct user_vfp {
    unsigned long long fpregs[32];
    unsigned long fpscr;
};

static inline void reg_write32(unsigned int Addr, unsigned int Value)
{
    volatile unsigned int  *TempAddr = (volatile unsigned int *)Addr;
     *TempAddr = Value;
}

static inline unsigned int reg_read32(unsigned int Addr)
{
    return *(volatile unsigned int *)Addr;
}

static inline void reg_write16(unsigned int Addr, unsigned short Value)
{
    volatile unsigned short *TempAddr = (volatile unsigned short *)Addr;
    *TempAddr = Value;
}

static inline unsigned short reg_read16(unsigned int Addr)
{
    return *(volatile unsigned short *) Addr;
}

static inline void reg_write8(unsigned int Addr, unsigned char Value)
{
    volatile unsigned char *TempAddr = (volatile unsigned char *)Addr;
    *TempAddr = Value;
}

static inline unsigned char reg_read8(unsigned int Addr)
{
    return *(volatile unsigned char *) Addr;
}



#endif
