#include "public.h"
#include "cortex_a9.h"
#include "tick.h"

// 获取CNTPCT的值
static inline unsigned long long int read_cntpct(void) 
{
    unsigned int low = 0, high = 0;
    asm volatile("mrrc p15, 0, %0, %1, c14" : "=r"(low), "=r"(high));
    return ((unsigned long long int)high << 32) | low;
}

// 获取CNTFRQ的值
static inline unsigned int read_cntfrq(void) 
{
    unsigned int freq = 0;
    asm volatile("mrc p15, 0, %0, c14, c0, 0" : "=r"(freq));
    return freq;
}

unsigned long long int current_usec(void) 
{
    static unsigned int freq = 0;
    unsigned long long int cnt = 0;
    if(freq == 0)
    {
        freq = read_cntfrq();
    }
    cnt = read_cntpct();
    return (cnt * 1000000) / freq;
}

int clock_gettime(struct timespec *ts)
{
    ts->tv_sec  =(unsigned int)(current_usec() / USEC_PER_SEC);
    ts->tv_nsec =(long)((current_usec() % USEC_PER_SEC) * NSEC_PER_USEC); 
    return 0;
}
