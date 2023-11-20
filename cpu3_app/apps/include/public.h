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

#endif
