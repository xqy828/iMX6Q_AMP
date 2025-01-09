#include "public.h"
#include "cortex_a9.h"
#include "usleep.h"

/* <<ARM® Cortex®-A9 MPCore Revision: r4p1 Technical Reference Manual>>
 * Global Timer is always clocked at half of the CPU frequency */
#define COUNTS_PER_USECOND  (ARM_CLK_ROOT / (2U*1000000U))
#define COUNTS_PER_SECOND   (ARM_CLK_ROOT / (2U))
#define COUNTS_PER_MSECOND  (ARM_CLK_ROOT / (2U*1000U))

/*
 * Global_Timer reg is not banked 
 * when this function is called by any one processor in a multi-processor environment, 
 * reference time will reset/lost for all processors.
 *
 * 
 * */

void SetGlobalTime(unsigned long long int value)
{
    volatile unsigned int data = 0;
        /* Disable Global Timer */    
    data = reg_read32((unsigned int)ARM_MP_ADDR + (unsigned int)Global_Timer + (unsigned int)Global_Timer_Control);
    data &=0xFFFFFFFE;
    reg_write32((unsigned int)ARM_MP_ADDR + (unsigned int)Global_Timer + (unsigned int)Global_Timer_Control,data);

    /* Updating Global Timer Counter Register */
    reg_write32((unsigned int)ARM_MP_ADDR + (unsigned int)Global_Timer + (unsigned int)Global_Timer_Counter_low, (unsigned int)value);
    reg_write32((unsigned int)ARM_MP_ADDR + (unsigned int)Global_Timer + (unsigned int)Global_Timer_Counter_high,(unsigned int)((unsigned int)(value >> 32)));

    /* Enable Global Timer */
    data = reg_read32((unsigned int)ARM_MP_ADDR + (unsigned int)Global_Timer + (unsigned int)Global_Timer_Control);
    data |=0x1;
    reg_write32((unsigned int)ARM_MP_ADDR + (unsigned int)Global_Timer + (unsigned int)Global_Timer_Control,data);
}


void GetGlobalTime(unsigned long long int *GlobalTime)
{
    unsigned int low = 0;
    unsigned int high = 0;
    /* Reading Global Timer Counter Register */
    do
    {
        high = reg_read32((unsigned int)ARM_MP_ADDR + (unsigned int)Global_Timer + (unsigned int)Global_Timer_Counter_high);
        low = reg_read32((unsigned int)ARM_MP_ADDR + (unsigned int)Global_Timer + (unsigned int)Global_Timer_Counter_low);
    } while(reg_read32((unsigned int)ARM_MP_ADDR + (unsigned int)Global_Timer + (unsigned int)Global_Timer_Counter_high) != high);
    *GlobalTime = (((unsigned long long int) high) << 32) | (unsigned long long int) low;
}

int usdelay(unsigned long long int useconds)
{
    unsigned long long int ullEnd=0, ullCur=0;

    GetGlobalTime(&ullCur);
    ullEnd = ullCur + (((unsigned long long int) useconds) * COUNTS_PER_USECOND);
    do
    {
        GetGlobalTime(&ullCur);
    } while (ullCur < ullEnd);
    return 0;
}

void SysTickElapseConvertToTime(tick_convert_type_t type,unsigned long long int tEnd,unsigned long long int tStart,unsigned int *ulTime)
{
    unsigned int tUsed = 0;
    unsigned long long int ullTickElapse = 0;
    
    if(tEnd > tStart)
    {
        ullTickElapse = tEnd-tStart;
    }
    else
    {
        ullTickElapse = U64_MAX - tStart + tEnd;
    }

    switch(type)
    {
        case T_US: 
            tUsed = (ullTickElapse)/(COUNTS_PER_USECOND);
        break;
        case T_MS:
            tUsed = (ullTickElapse)/(COUNTS_PER_MSECOND);
        break;
        case T_S:
            tUsed = (ullTickElapse)/(COUNTS_PER_SECOND);
        break;
        default : 
            disp("invalid time type \n");
            tUsed = 0;
        break;
    }

    *ulTime = tUsed;
}
