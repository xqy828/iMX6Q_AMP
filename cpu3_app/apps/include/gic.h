#ifndef GIC_H_
#define GIC_H_
#include "exception.h"

#define IMX6Q_SCUGIC_MAX_INTR_NUM      (160) // 0-15 SGI , 16:31 PPI , 32:159 SPI
#define DEFAULT_IRQ_PRIORITY           (160)

typedef void (*InterruptHandler)(void *data);

typedef struct
{
    InterruptHandler Handler;
    void *CallBack;
}ScuGic_VectorTableEntry;

void ScuGic_Stop(unsigned int cpuid);
void DistributorInit(unsigned int cpuid);
void Distributor_SetIrqTargetCpu(unsigned int irq,unsigned int cpuid);
void Distributor_SetIrqPriority(unsigned int irq,unsigned int priority);
void Distributor_SetIrqTriggerType(unsigned int irq,unsigned int ulTrigger);
void Distributor_SetIrqEnable(unsigned int irq);

void CpuInterface_SetPriorityMask(unsigned int  priority);
void CpuInterface_SetCtrl(unsigned int ulCtlr);
void CpuInerface_SetBinaryPoint(unsigned int ulvalue);

void CpuCore_SetCpsrEnableIrq(void);
void CpuCore_SetCpsrEnableFiq(void);

void RegisterIrqHandler(unsigned int Irq,InterruptHandler IrqHandle);
void RegisterScuGicHandler(ExceptionHandler Handle,void *data);
void Gic_InterruptHandler(void);

void local_irq_enable(void);
void local_irq_disable(void);


#endif
