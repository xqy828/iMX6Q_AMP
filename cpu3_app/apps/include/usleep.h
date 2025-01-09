#ifndef USLEEP_H_
#define USLEEP_H_

typedef enum tick_convert_type 
{
    T_US,
    T_MS,
    T_S,
}tick_convert_type_t;

int usdelay(unsigned long long int useconds);
void GetGlobalTime(unsigned long long int *GlobalTime);
void SetGlobalTime(unsigned long long int value);
void SysTickElapseConvertToTime(tick_convert_type_t type,unsigned long long int tEnd,unsigned long long int tStart,unsigned int *ulTime);


#endif

