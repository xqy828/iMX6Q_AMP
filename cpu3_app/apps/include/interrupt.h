#ifndef INTERRUPT_H_
#define INTERRUPT_H_

void SCU_TimerSetupInterrupt(void);
int SCU_SendSgi2Cpu0(void);
#endif
