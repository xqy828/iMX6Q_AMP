#include <stdio.h>
#include <string.h>
#include "cortex_a9.h"
#include "public.h"
#include "mmu.h"
#include "arm_cache.h"
#include "usleep.h"
#include "interrupt.h"

extern unsigned int TestNeon(void);
extern void TestRoundData(unsigned int Row,unsigned int Line);
extern unsigned int softuart[2];
unsigned int process = 0xdeadbeef;

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
    disp("process debug addr:0x%x\n",&process);
    disable_mmu();// if enabled
    cpsr_reg = read_cpsr_reg();
    disp("cpsr reg = 0x%08lx\n",cpsr_reg);
    _arm_mrc(15, 0, vbar_reg, 12, 0, 0);
    disp("vbar reg = 0x%08lx\n",vbar_reg);
    disp_scu_all_regs();
    
    invalidate_tlb();
    
    arm_icache_enable();
    arm_dcache_enable();

    arm_icache_invalidate();
    arm_dcache_invalidate();
    arm_branch_target_cache_invalidate();
    arm_branch_prediction_enable();        
    /* enable scu Cache/TLB maintenance broadcast L2 cache and DDR 
     * The SCU maintains coherency between the L1 data cache of each core
     * Maintain data cache coherency between the Cortex-A9 processors.
     * Initiate L2 AXI memory accesses.
     * Arbitrate between Cortex-A9 processors requesting L2 accesses.
     * Manage ACP accesses.
     *
     */
    scu_enable();
    scu_join_smp();
    scu_enable_maintenance_broadcast(); 

    mmu_init();
    disp("Enable MMU \n");
    SetTlbAttributes((unsigned int)&softuart[0],0x1,0x32);
    mmu_enable();
    disp("Enable SIMD VFP \n");
    Enable_SIMD_VFP();
    SetGlobalTime(0);
    disp("Neon Test ...\n");
    TestNeon();
    disp("Normal Distribution Random number Test ...\n");
    TestRoundData(10,5);
    SCU_TimerSetupInterrupt();
    for(;;)
    {
        disp("run times:0x%08x.\n",cnt);
        //data_abort_test();
        //prefectch_abort_test();
        cnt++;
        usdelay(1000*1000);
    }
}
