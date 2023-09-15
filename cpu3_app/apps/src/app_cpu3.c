#include <stdio.h>
#include <string.h>
__attribute__ ((section (".cpu3softuart")))  unsigned int softuart[2] = {[0 ... 1] = 0x0};

#define COMM_TX_FLAG (softuart[0])
#define COMM_TX_DATA (softuart[1])

const unsigned int PLL1_CLK = 792000000;

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

void my_print(const char *ptr)
{
    while(*ptr)
    {
        myoutbyte(*ptr++);
    }
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

__attribute__ ((section (".cpu3main"))) void main(void)
{
    const char Date[12] = __DATE__;
    const char Time[9] = __TIME__;
    volatile unsigned int cnt = 0;
    char uart[256];

    memset(uart,0,256*sizeof(char));
    sprintf(uart,"[-CPU3-]:Build Time:%s-%s.\n",Date,Time);
    my_print(uart);
    for(;;)
    {
        sprintf(uart,"[-CPU3-]:run times:0x%08x.\n",cnt);
        my_print(uart);
        cnt++;
        delay(30);
    }
}
