#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/select.h>
#include "public.h"
#include "ampCtrl.h"
#include "kernelmap.h"

char *str = "App start ...";
extern int SoftUartInit();
const char Date[12] = __DATE__;
const char Time[9] = __TIME__;

S32 main (void)
{
    printf("\n%s\n",str);
    printf("Build Time:%s-%s.\n",Date,Time);
    KernelMmapInit();
    InitCpu3();
    SoftUartInit();
    for(;;)
    {
        msleep(30);
    }
    return RET_OK;
}


