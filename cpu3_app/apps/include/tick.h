#ifndef TICK_H_
#define TICK_H_

#define NSEC_PER_SEC          1000000000UL /* Seconds */
#define USEC_PER_SEC             1000000UL
#define MSEC_PER_SEC                1000UL

#define NSEC_PER_MSEC            1000000UL /* Milliseconds */
#define USEC_PER_MSEC               1000UL
#define NSEC_PER_USEC               1000UL /* Microseconds */



struct timespec
{
    unsigned int tv_sec;                   /* Seconds */
    long   tv_nsec;                  /* Nanoseconds */
};

int clock_gettime(struct timespec *ts);

#endif

