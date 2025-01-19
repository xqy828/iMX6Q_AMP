#include "public.h"
#include "cortex_a9.h"
#include "gic.h"
#include "interrupt.h"
#include "PrivateTimer.h"

volatile int TimerExpired = 0;
void SCU_TimerHandler(void *arg)
{
    disp("in scu timer isr %d\n",TimerExpired++);
    ScuTimer_ClearInterruptStatus();
}

void SCU_TimerSetupInterrupt(void)
{
    const unsigned char ucRisingEdge = 3;
    const unsigned int cpuid = 3;
    
    DistributorInit(cpuid);
    /* 256 level */
    CpuInterface_SetPriorityMask(0xff);
    /* gic v1 only enable group 1&0 and AckCtl */
    CpuInterface_SetCtrl(0x07);
    Distributor_SetIrqPriority(SCU_TMR_INT_ID,DEFAULT_IRQ_PRIORITY - 1);
    Distributor_SetIrqTriggerType(SCU_TMR_INT_ID,ucRisingEdge);
    RegisterIrqHandler(SCU_TMR_INT_ID,(InterruptHandler)SCU_TimerHandler);
    RegisterScuGicHandler((ExceptionHandler)Gic_InterruptHandler,NULL);
    ScuTimer_EnableAutoReload();
    ScuTimer_SetPrescaler(0);
    ScuTimer_LoadTimer((ARM_CLK_ROOT / (2U)) * 2); //2s
    ScuTimer_Start();
    Distributor_SetIrqEnable(SCU_TMR_INT_ID);
    ScuTimer_ClearInterruptStatus();
    ScuTimer_EnableInterrupt();
    CpuCore_SetCpsrEnableIrq();
}


