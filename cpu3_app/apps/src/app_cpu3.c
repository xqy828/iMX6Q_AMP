#include <stdio.h>
#include <string.h>
#include "cortex_a9.h"
#include "public.h"
__attribute__ ((section (".cpu3softuart")))  unsigned int softuart[2] = {[0 ... 1] = 0x0};

#define COMM_TX_FLAG (softuart[0])
#define COMM_TX_DATA (softuart[1])

const unsigned int PLL1_CLK = 792000000;
extern unsigned int TestNeon(void);
extern void TestRoundData(unsigned int Row,unsigned int Line);

void myPutChar(char c)
{
    while(COMM_TX_FLAG);//wait other cpu consume previous value
    COMM_TX_DATA = (unsigned int)c;
    COMM_TX_FLAG = 1;
}

void myoutbyte(char c)
{
    myPutChar(c);
}

void delay_short(volatile unsigned int n)
{
    while(n--)
    {

    }
}

void delay(volatile unsigned int n)
{
    while(n--)
    {
        delay_short(0xbbccf);
    }
}

int _write(int fd,char *ptr,int len)
{
    int i = 0;
    UNUSED_PARA(fd);
    for (i = 0; i < len; i++)
    {
        myoutbyte(*ptr);
        ptr++;
    }
    return len;
}

double gdPi = 3.141592654;
__attribute__ ((section (".cpu3main"))) void main(void)
{
    const char Date[12] = __DATE__;
    const char Time[9] = __TIME__;
    volatile unsigned int cnt = 0;
    
    printf("[-CPU3-]:Build Time:%s-%s.\n",Date,Time);
    printf("[-CPU3-]:float test pi = %lf\n",gdPi);
    printf("[-CPU3-]:Enable SIMD VFP \n");
    Enable_SIMD_VFP();
    printf("[-CPU3-]:Neon Test ...\n");
    TestNeon();
    printf("[-CPU3-]:Normal Distribution Random number Test ...\n");
    TestRoundData(10,5);
    for(;;)
    {
        printf("[-CPU3-]:run times:0x%08x.\n",cnt);
        cnt++;
        delay(30);
    }
}
