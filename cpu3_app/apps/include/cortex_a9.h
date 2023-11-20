
#if !defined(__CORTEX_A9_H__)
#define __CORTEX_A9_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

void Enable_SIMD_VFP(void);
int get_cpuid(void);
#endif
