#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "public.h"

extern void vfp_read_d02d31(struct user_vfp *ctx);

float Q_rsqrt(float number)
{
    long int i;
    float x2,y;
    const float threehalfs = 1.5f;
    x2 = number * 0.5f;
    y = number;
    i = *(long*)&y;
    i = 0x5f3759df - (i >> 1);
    y = *(float*)&i;
    y = y * (threehalfs - (x2 * y * y));
    return y;
}

double Uniform(double MinLimit, double MaxLimit ,unsigned long int *Seed) 
{
    double t;
    *Seed = 2045 * (*Seed) + 1;
    *Seed = *Seed - (*Seed / 1048576) * 1048576;
    t = (*Seed) / 1048576.0;
    t = MinLimit + (MaxLimit - MinLimit) *t;
    return (t);
}

double Gauss(double Mean , double Sigma , unsigned long int *Seed)
{
    double i;
    double x , y;
    for(i = 0,x = 0 ;i < 12; i++)
    {
        x += Uniform(0.0,1.0,Seed);
    }
    x = x - 6.0;
    y = Mean + x * Sigma;
    return (y);
}

/* 
 * math test 
 */

void TestRoundData(unsigned int Row,unsigned int Line)
{
    unsigned int Rindex,Lindex;
    double x = 0.0;
    double mean = 0.0;
    double sigma = 1.0;
    unsigned long int Seed = 13579;
    for(Lindex = 0;Lindex < Line; Lindex++)
    {
        for(Rindex = 0;Rindex < Row; Rindex++)
        {
            x = Gauss(mean,sigma,&Seed);
            printf("%13.7f",x);
        }
        printf("\n");
    }
}

int test_vfp(void)
{
    struct user_vfp ctx;
    const double val_pi = 3.14;
    const double val_e  = 2.718;

    asm __volatile__ (
        "vldr d0, %0\n"
        "vldr d31, %1\n"
        :
        : "m"(val_pi), "m"(val_e)
        : "d0", "d31", "memory"
    );

    vfp_read_d02d31(&ctx);
    
    double d0_val, d31_val;
    memcpy(&d0_val, &ctx.fpregs[0], sizeof(double));
    memcpy(&d31_val, &ctx.fpregs[31], sizeof(double));
    
    disp("D0  = %.6f\n", d0_val);
    disp("D31 = %.6f\n", d31_val);
    disp("FPSCR = 0x%08x\n", ctx.fpscr);
    
    return 0;
}




