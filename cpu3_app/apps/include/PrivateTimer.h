#ifndef PRIVATETIMER_H_
#define PRIVATETIMER_H_

void ScuTimer_Start(void);
void ScuTimer_Stop(void);
void ScuTimer_EnableAutoReload(void);
void ScuTimer_EnableInterrupt(void);
void ScuTimer_LoadTimer(unsigned int value);
void ScuTimer_GetCounter(unsigned int *pValue);
void ScuTimer_ClearInterruptStatus(void);

#endif

