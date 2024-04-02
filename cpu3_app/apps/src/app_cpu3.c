#include <stdio.h>
#include <string.h>
#include "cortex_a9.h"
#include "public.h"
#include "mmu.h"

const unsigned int PLL1_CLK = 792000000;
extern unsigned int TestNeon(void);
extern void TestRoundData(unsigned int Row,unsigned int Line);

void delay_short(volatile unsigned int n)
{
    while(n--)
    {

    }
}

void delay(volatile unsigned int n)
{
    while(n--)
    {
        delay_short(0xbbccf);
    }
}

void data_abort_test(void)
{
    unsigned int *p = (unsigned int *)0x79880001;
    *p = 1;
}

void prefectch_abort_test(void)
{
    unsigned int badaddr = 0x79000000;
    asm volatile(
        "mov r5,%0\n"
        "mov pc,r5\n"
        :
        : "r"(badaddr)
        : "r5"
                );
}


double gdPi = 3.141592654;
__attribute__ ((section (".cpu3main"))) void main(void)
{
    const char Date[12] = __DATE__;
    const char Time[9] = __TIME__;
    volatile unsigned int cnt = 0;
    unsigned int vbar_reg = 0;
    unsigned int cpsr_reg = 0;
    disp("Build Time:%s-%s.\n",Date,Time);
    disp("float test pi = %lf\n",gdPi);
    disp("Enable SIMD VFP \n");
    Enable_SIMD_VFP();
    disp("Neon Test ...\n");
    TestNeon();
    disp("Normal Distribution Random number Test ...\n");
    TestRoundData(10,5);
    cpsr_reg = read_cpsr_reg();
    disp("cpsr reg = 0x%08lx\n",cpsr_reg);
    _arm_mrc(15, 0, vbar_reg, 12, 0, 0);
    disp("vbar reg = 0x%08lx\n",vbar_reg);
    disp_scu_all_regs();
    mmu_table_init();
    Test_VirtualMMU(0xdeadbeef);
    for(;;)
    {
        disp("run times:0x%08x.\n",cnt);
        //data_abort_test();
        //prefectch_abort_test();
        cnt++;
        delay(30);
    }
}
