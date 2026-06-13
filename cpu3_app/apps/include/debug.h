#ifndef DEBUG_H_
#define DEBUG_H_
#include "public.h"
void dump_stack(void);
void Test_dump_stack(void);
int do_coredump(void);
int coredump_initialize(void);
void Test_coredump(void);
void dump_stack_isr(struct pt_regs *regs);
#endif

