#include "asm_defines.h"
#include "public.h"
#include "cortex_a9.h"
#include "gic.h"
#include "exception.h"

/* 
 * in this project CPU3 is power on other cpu core ,  
 * so current cpu only configure cpu interface .  
 * PL390 GIC V1
 * */

ScuGic_VectorTableEntry gHandlerTable[IMX6Q_SCUGIC_MAX_INTR_NUM] = {[0 ... (IMX6Q_SCUGIC_MAX_INTR_NUM - 1)] = {(InterruptHandler)NULL,NULL}};

void  RegisterIrqHandler(unsigned int Irq,InterruptHandler IrqHandle)
{
    gHandlerTable[Irq].Handler = IrqHandle;
}

void ScuGic_Stop(unsigned int cpuid)
{
    unsigned int gicStatus = 0;
    unsigned int IntrId = 0;
    unsigned int TargetCpu = 0;
    unsigned int localCpuMask = 0;
    unsigned int localCpu = 0;
    unsigned int CpuOffset = 0;
    unsigned char ucDisableDistributor = 1;
    gicStatus = reg_read32((unsigned int)ARM_MP_ADDR + (unsigned int)Interrupt_Distributor + (unsigned int)GICD_CTLR);
    /* If distributor is already disabled, no need to do anything */
    // GIC V2
    if((gicStatus & 0x3) == 0)// EnableGrp1 | EnableGrp0 secure and non-secure
    {
        return;
    }
    
    localCpu = 1 << cpuid;// cpu 3 

    localCpuMask = (localCpu << 0) | (localCpu << 8) | (localCpu << 16) | (localCpu << 24); 
    /* in AMP mode , this is taken care by master cpu , so I think should remove cpu target ,no need to disable gic distributor */

    for(IntrId = 0; IntrId < IMX6Q_SCUGIC_MAX_INTR_NUM; IntrId = IntrId + 4)
    {
        CpuOffset = (IntrId / 4) * 4;
        TargetCpu = reg_read32((unsigned int)ARM_MP_ADDR + (unsigned int)Interrupt_Distributor + (unsigned int)GICD_ITARGETSRn + CpuOffset);   
        if((TargetCpu != localCpuMask) && (TargetCpu != 0))
        {
            disp("other cpu is also configured to target register \n");
            ucDisableDistributor = 0;
        }
        /* clear all cpu 3 from target reg */
        TargetCpu &= (~localCpuMask);
        reg_write32((unsigned int)ARM_MP_ADDR + (unsigned int)Interrupt_Distributor + (unsigned int)GICD_ITARGETSRn + CpuOffset,TargetCpu);
    }
    /* in AMP mode , this is taken care by master cpu , so I think should do nothing here */
    if(ucDisableDistributor  == 1)
    {
        disp("disable gic\n");
        for(IntrId = 0; IntrId < IMX6Q_SCUGIC_MAX_INTR_NUM; IntrId = IntrId + 32)
        {
            /* clean all pending interrupt */
            reg_write32((unsigned int)ARM_MP_ADDR + (unsigned int)Interrupt_Distributor + (unsigned int)GICD_ICPENDRn + (IntrId / 32) * 4,0xFFFFFFFF);
            /* clear all enable interrupt */
            reg_write32((unsigned int)ARM_MP_ADDR + (unsigned int)Interrupt_Distributor + (unsigned int)GICD_ICENABLERn + (IntrId / 32) * 4,0xFFFFFFFF);
        }
        reg_write32((unsigned int)ARM_MP_ADDR + (unsigned int)Interrupt_Distributor + (unsigned int)GICD_CTLR,0x0);
    }
}


void DistributorInit(unsigned int cpuid)
{
    /* in AMP mode , this is taken care by master cpu , so I think should do nothing here */
    UNUSED_PARA(cpuid);
}

/*
 * for SPI interrupt
 *
 */

void Distributor_SetIrqTargetCpu(unsigned int irq,unsigned int cpuid)
{
    unsigned int cpumask = 1 << cpuid;
    unsigned int offset  = irq & 0x3;
    unsigned int regvalue  = 0;
 
    regvalue = reg_read32((unsigned int)ARM_MP_ADDR + (unsigned int)Interrupt_Distributor + (unsigned int)GICD_ITARGETSRn + (irq / 4) * 4);   
    regvalue = (regvalue & (~(0xff << (offset*8))));
    regvalue |= ((cpumask) << (offset*8));
    reg_write32((unsigned int)ARM_MP_ADDR + (unsigned int)Interrupt_Distributor + (unsigned int)GICD_ITARGETSRn + (irq / 4) * 4,regvalue);
}

void Distributor_SetIrqPriority(unsigned int irq,unsigned int priority)
{
    unsigned int regvalue = 0;
    unsigned int prioritytemp = 0;
    
    regvalue = reg_read32((unsigned int)ARM_MP_ADDR + (unsigned int)Interrupt_Distributor + (unsigned int)GICD_IPRIORITYRn + (irq / 4) * 4);   
    prioritytemp = priority & 0xff;
    regvalue &=~(0xff << ((irq % 4) * 8));
    regvalue |= prioritytemp << ((irq % 4) * 8);
    reg_write32((unsigned int)ARM_MP_ADDR + (unsigned int)Interrupt_Distributor + (unsigned int)GICD_IPRIORITYRn + (irq / 4) * 4,regvalue);
}

void Distributor_SetIrqTriggerType(unsigned int irq,unsigned int ulTrigger)
{
    unsigned int regvalue = 0;
    regvalue = reg_read32((unsigned int)ARM_MP_ADDR + (unsigned int)Interrupt_Distributor + (unsigned int)GICD_ICFGRn + (irq / 16) * 4);   
    regvalue &=~(0x3 << ((irq % 16) * 8));
    regvalue |= (ulTrigger & 0x3) << ((irq % 16) * 2);
    reg_write32((unsigned int)ARM_MP_ADDR + (unsigned int)Interrupt_Distributor + (unsigned int)GICD_ICFGRn + (irq / 16) * 4,regvalue);
}

void Distributor_SetIrqEnable(unsigned int irq)
{
    unsigned int irqmask = 0;
    irqmask = 0x1 << (irq % 32);// irq & 0x1f
    reg_write32((unsigned int)ARM_MP_ADDR + (unsigned int)Interrupt_Distributor + (unsigned int)GICD_ISENABLERn + (irq / 32) * 4,irqmask);
}

void CpuInterface_SetPriorityMask(unsigned int  priority)
{
    unsigned int uldata = 0;
    uldata = priority & 0xFF;
    reg_write32((unsigned int)ARM_MP_ADDR + (unsigned int)Interrupt_Controller_Interfaces + (unsigned int)GICC_PMR,uldata);
}

/*
 * GIC V1
 * if CPU running in both secure and non-secure , the GICC_CTLR should be : 
 * 1. Set FIQen=1 to use FIQ for secure interrupts. Signal Group 0 interrupts using the FIQ signal
 * 2. program AckCtl 
 * 3. Program the SBPR bit to select the binary pointer behavior
 * 4. Set EnableS = 1 to enable secure interrupts
 * 5. Set EnbleNS = 1 to enable non secure interrupts
 */

void CpuInterface_SetCtrl(unsigned int ulCtlr)
{
    reg_write32((unsigned int)ARM_MP_ADDR + (unsigned int)Interrupt_Controller_Interfaces + (unsigned int)GICC_CTLR,ulCtlr);
}

void CpuInerface_SetBinaryPoint(unsigned int ulvalue)
{
    unsigned int uldata = 0;
    uldata = ulvalue & 0x7;
    reg_write32((unsigned int)ARM_MP_ADDR + (unsigned int)Interrupt_Controller_Interfaces + (unsigned int)GICC_BPR,uldata);
}

void CpuCore_SetCpsrEnableIrq(void)
{
    unsigned int cpsr_reg = 0;
    asm volatile(
        "mrs %0, cpsr\n"
        : "=r" (cpsr_reg)
        :
        :
        );
    cpsr_reg &=~(I_BIT);
       asm volatile(
        "msr cpsr,%0\n"
        :
        : "r" (cpsr_reg)
        :
        );
}

void CpuCore_SetCpsrEnableFiq(void)
{
    unsigned int cpsr_reg = 0;
    asm volatile(
        "mrs %0, cpsr\n"
        : "=r" (cpsr_reg)
        :
        :
        );
    cpsr_reg &=~(F_BIT);
       asm volatile(
        "msr cpsr,%0\n"
        :
        : "r" (cpsr_reg)
        :
        );
}

void Gic_InterruptHandler(void)
{
    unsigned int IrqID = 0;
    unsigned int IntIDFull;
    ScuGic_VectorTableEntry *TablePtr;
    IntIDFull = reg_read32((unsigned int)ARM_MP_ADDR + (unsigned int)Interrupt_Controller_Interfaces + (unsigned int)GICC_IAR);   
    IrqID = IntIDFull & 0x3ff; 
    if (IrqID >= IMX6Q_SCUGIC_MAX_INTR_NUM)
    {
        disp("read irq id abnormal \n");
        reg_write32((unsigned int)ARM_MP_ADDR + (unsigned int)Interrupt_Controller_Interfaces + (unsigned int)GICC_EOIR,IntIDFull);   
        return;
    }

    TablePtr = &gHandlerTable[IrqID];
    if(TablePtr != NULL)
    {
        TablePtr->Handler(TablePtr->CallBack);
    }
    reg_write32((unsigned int)ARM_MP_ADDR + (unsigned int)Interrupt_Controller_Interfaces + (unsigned int)GICC_EOIR,IntIDFull);   
}

void RegisterScuGicHandler(ExceptionHandler Handle,void *data)
{
    ExceptionRegisterHandler(EXCEPTION_ID_IRQ_INT,Handle,data);
}

void local_irq_enable(void)
{
    asm volatile(
        "cpsie i\n"
        : 
        :
        : "memory"
        );
}

void local_irq_disable(void)
{
    asm volatile(
        "cpsid i\n"
        : 
        :
        : "memory"
        );
}







