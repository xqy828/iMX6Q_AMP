/*
 * Copyright (c) 2012 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <byteswap.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include "public.h"
#include "ampCtrl.h"

#define COMM_TX_FLAG_OFFSET     0x00
#define COMM_TX_DATA_OFFSET     0x04

#define PAGE_SIZE ((size_t)getpagesize())
#define PAGE_MASK ((uint64_t)(long)~(PAGE_SIZE - 1))

#define MAX_STR 256

UADDR CPU3_SOFT_UART_BASE = 0;
UADDR VirCpu3SoftUartBase = 0;

void *softuart(void)
{
    uint32_t value=0;
    uint32_t flag=0;
    uint8_t str[MAX_STR];
    int rcnt = 0;
    printf("softuart thread start ...\r\n");
    memset(str,0,MAX_STR*sizeof(uint8_t));
    for(;;)
    {//read
        if( (flag = *(volatile uint32_t *)(VirCpu3SoftUartBase + COMM_TX_FLAG_OFFSET)) )
        {
            value = *(volatile uint32_t *)(VirCpu3SoftUartBase + COMM_TX_DATA_OFFSET);

            //process non-string type data
            if(flag > 1) 
            {
                printf("CPU3: 0x%08x = 0x%08x\n", (uint32_t)(VirCpu3SoftUartBase + COMM_TX_DATA_OFFSET), value);
                //process string type data
            }
            else 
            {
                if(rcnt < MAX_STR) 
                {
                    str[rcnt++] = (uint8_t)value;
                }
                if(value == '\n') 
                {
                    if(rcnt != 0) 
                    {
                        str[rcnt-1] = '\0';
                    }
                    else 
                    {
                        str[0] = '\0';
                    }
                    printf("%s\n", str);
                    rcnt = 0;
                }
            }
            *(volatile uint32_t *) (VirCpu3SoftUartBase + COMM_TX_FLAG_OFFSET) = 0;
        }
    }
}


int SoftUartInit(void)
{
    int rc = 0;
    int fd = 0;
    pthread_t pid;
    volatile uint8_t *mm=NULL;
    U32 uloffset = 0;
    U32 ulPhyBase = 0;

    fd = open("/dev/mem", O_RDWR|O_SYNC);
    if (fd < 0) {
        fprintf(stderr, "open(/dev/mem) failed (%d)\n", errno);
        return 1;
    }
    rc = getCpu3SectionAddr(".cpu3softuart",&CPU3_SOFT_UART_BASE);
    if(rc != RET_OK)
    {
        printf("get cpu3 soft uart addr failed \r\n");
        return RET_NOK;
    }
    printf("cpu3 soft uart addr:0x%lx\r\n",CPU3_SOFT_UART_BASE);

    uloffset = CPU3_SOFT_UART_BASE % PAGE_SIZE;
    ulPhyBase  = CPU3_SOFT_UART_BASE - uloffset;

    mm = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, ulPhyBase);
    if (mm == MAP_FAILED) {
        fprintf(stderr, "mmap64(0x%x@0x%lx) failed (%d:%s)\n",
                PAGE_SIZE, (UADDR)(CPU3_SOFT_UART_BASE), errno,strerror(errno));
        return 1;
    }
    VirCpu3SoftUartBase = (UADDR)mm + uloffset;

    printf("cpu3 soft uart vir addr: 0x%lx \n", VirCpu3SoftUartBase);
    close(fd);

    rc = pthread_create(&pid,NULL,(void*)softuart,NULL);
    if(rc != 0)
    {
        perror("soft urat thread create fail \n");
        return RET_NOK;
    }
    rc = pthread_setname_np(pid,"t_softuart");
    if(rc != 0)
    {
        perror("t_softuart thread name  set fail \n");
        return RET_NOK;
    }
    return RET_OK;
}
