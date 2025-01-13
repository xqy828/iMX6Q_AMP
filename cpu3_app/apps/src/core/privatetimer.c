#include "public.h"
#include "cortex_a9.h"
#include "PrivateTimer.h"

//load = (ARM_CLK_ROOT / (2*1000)); // 1ms RTOS tick
 
void ScuTimer_Start(void)
{
    unsigned int reg = 0;
    reg = reg_read32((unsigned int)ARM_MP_ADDR + (unsigned int)Private_Timers_and_Watchdogs + (unsigned int)Prv_Timer_Control);
    reg |= PRV_TIMER_CTRL_TIMER_ENABLE;
    reg_write32((unsigned int)ARM_MP_ADDR + (unsigned int)Private_Timers_and_Watchdogs + (unsigned int)Prv_Timer_Control,reg);
}

void ScuTimer_Stop(void)
{
    unsigned int reg = 0;
    reg = reg_read32((unsigned int)ARM_MP_ADDR + (unsigned int)Private_Timers_and_Watchdogs + (unsigned int)Prv_Timer_Control);
    reg &= (unsigned int)(~PRV_TIMER_CTRL_TIMER_ENABLE);
    reg_write32((unsigned int)ARM_MP_ADDR + (unsigned int)Private_Timers_and_Watchdogs + (unsigned int)Prv_Timer_Control,reg);
}

void ScuTimer_EnableAutoReload(void)
{
    unsigned int regdata = 0;
    regdata = reg_read32((unsigned int)ARM_MP_ADDR + (unsigned int)Private_Timers_and_Watchdogs + (unsigned int)Prv_Timer_Control);

    regdata |= PRV_TIMER_CTRL_AUTO_RELOAD;
    reg_write32((unsigned int)ARM_MP_ADDR + (unsigned int)Private_Timers_and_Watchdogs + (unsigned int)Prv_Timer_Control,regdata);
}

void ScuTimer_EnableInterrupt(void)
{
    unsigned int regdata = 0;
    regdata = reg_read32((unsigned int)ARM_MP_ADDR + (unsigned int)Private_Timers_and_Watchdogs + (unsigned int)Prv_Timer_Control);

    regdata |= PRV_TIMER_CTRL_IRQ_ENABLE; 
    reg_write32((unsigned int)ARM_MP_ADDR + (unsigned int)Private_Timers_and_Watchdogs + (unsigned int)Prv_Timer_Control,regdata);
}


void ScuTimer_LoadTimer(unsigned int value)
{
    reg_write32((unsigned int)ARM_MP_ADDR + (unsigned int)Private_Timers_and_Watchdogs + (unsigned int)Prv_Timer_Load,value);
}

void ScuTimer_GetCounter(unsigned int *pValue)
{
    unsigned int value = 0;
    value = reg_read32((unsigned int)ARM_MP_ADDR + (unsigned int)Private_Timers_and_Watchdogs + (unsigned int)Prv_Timer_Count);
    *pValue = value;
}

/**
 *	Clears the event register
 */
void ScuTimer_ClearInterruptStatus(void)
{
    /* clear interrupt pending */
    reg_write32((unsigned int)ARM_MP_ADDR + (unsigned int)Private_Timers_and_Watchdogs + (unsigned int)Prv_Timer_Intr,PRV_TIMER_INTR_EVENT);
}







