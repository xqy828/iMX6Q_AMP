#ifndef PUBLIC_H_ 
#define PUBLIC_H_
#include <stdio.h>
#include <string.h>
#include "cortex_a9.h"

#define UNUSED_PARA(para)   ((void)(para)) 
#define disp(format, ...)  \
do{\
    unsigned int cpuid = 0xf;\
    cpuid = get_cpuid();\
    printf("[\033[0;31m-CPU%d-\033[0m]:%s-(%05d)]"format,cpuid,__FUNCTION__,__LINE__,##__VA_ARGS__);\
}while(0)

#define U32_MAX         (4294967295u)                                  
#define U64_MAX         (18446744073709551615u)

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
