#include "public.h"
#include "cortex_a9.h"
#include "generic_timer.h"

// 获取CNTPCT的值
static inline unsigned long long int read_cntpct(void) 
{
    unsigned int low = 0, high = 0;
    asm volatile("isb");
    asm volatile("mrrc p15, 0, %0, %1, c14" : "=r"(low), "=r"(high));
    return ((unsigned long long int)high << 32) | low;
}

// 获取CNTFRQ的值
static inline unsigned int read_cntfrq(void) 
{
    unsigned int freq = 0;
    asm volatile("isb");
    asm volatile("mrc p15, 0, %0, c14, c0, 0" : "=r"(freq));
    return freq;
}

__attribute__((weak)) unsigned long long int current_usec(void) 
{
    static unsigned int freq = 0;
    unsigned long long int cnt = 0;
    disp("freq:%d,cnt:%lld\n",freq,cnt);
  
    if(freq == 0)
    {
        freq = read_cntfrq();
    }
    cnt = read_cntpct();
    return (cnt * 1000000) / freq;
}

__attribute__((weak)) int clock_gettime(struct timespec *ts)
{
    unsigned long long int usec = 0;
    usec = current_usec(); 
    ts->tv_sec  =(unsigned int)(usec / USEC_PER_SEC);
    ts->tv_nsec =(long)((usec % USEC_PER_SEC) * NSEC_PER_USEC); 
    return 0;
}
