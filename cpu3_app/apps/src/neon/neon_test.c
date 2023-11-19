#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "arm_neon.h"

#define TEST_DATA_LEN (8192)
float CosParam[TEST_DATA_LEN] = {
                                #include "cosParam.txt"
                            }; 
float SinParam[TEST_DATA_LEN] = {
                                #include "sinParam.txt"
                            }; 

short dataIn_I[TEST_DATA_LEN] = {
                                #include "DATA_IN_I.txt"
                            };
short dataIn_Q[TEST_DATA_LEN] = {
                                #include "DATA_IN_Q.txt"
                            };

short dataOut_I[TEST_DATA_LEN] = {0};
short dataOut_Q[TEST_DATA_LEN] = {0};

int32_t datatest_in[4] = {25791,531569, -2334455,-11515};
short datatest_out[4] = {0};
 
unsigned int ulTestDataLen = 8192;
unsigned int TestNeon(void)
{
    unsigned int ucLen  = ulTestDataLen;
    unsigned int ucIndx = 0;
    
    int32x4_t datatest_neon;
    int16x4_t dataInNeon_I;
    int16x4_t dataInNeon_Q;
    int32x4_t dataITmp;
    int32x4_t dataQTmp;
    float32x4_t sindata;
    float32x4_t cosdata;

    float32x4_t dataOutITmp;
    float32x4_t dataOutQTmp;

    for(ucIndx = 0;ucIndx < ucLen;ucIndx++)
    {
        dataOut_I[ucIndx] = dataIn_I[ucIndx]*SinParam[ucIndx] - dataIn_Q[ucIndx]*CosParam[ucIndx];
        dataOut_Q[ucIndx] = dataIn_Q[ucIndx]*SinParam[ucIndx] + dataIn_I[ucIndx]*CosParam[ucIndx];
    }
    
    for(ucIndx = 0;ucIndx < ucLen;ucIndx++)
    {
        printf("I:%6d-Q:%6d|",dataOut_I[ucIndx],dataOut_Q[ucIndx]);
    }

    printf("\n");
    printf("[-CPU3-]:C Intrinsics done \n");

    for(ucIndx = 0;ucIndx < ucLen /4 ; ucIndx++)
    {
        dataInNeon_I = vld1_s16(&dataIn_I[ucIndx*4]);
        dataInNeon_Q = vld1_s16(&dataIn_Q[ucIndx*4]);

        sindata = vld1q_f32(&SinParam[ucIndx*4]);
        cosdata = vld1q_f32(&CosParam[ucIndx*4]);
    
        dataITmp =  vmovl_s16(dataInNeon_I);
        dataQTmp =  vmovl_s16(dataInNeon_Q);

       dataOutITmp = (vsubq_f32(vmulq_f32(vcvtq_f32_s32(dataITmp),sindata),vmulq_f32(vcvtq_f32_s32(dataQTmp),cosdata)));
       dataOutQTmp = (vaddq_f32(vmulq_f32(vcvtq_f32_s32(dataQTmp),sindata),vmulq_f32(vcvtq_f32_s32(dataITmp),cosdata)));
       
       //vst1_s16(&dataOut_I[ucIndx*4],vmovn_s32(vcvtq_s32_f32(dataOutITmp)));
       //vst1_s16(&dataOut_Q[ucIndx*4],vmovn_s32(vcvtq_s32_f32(dataOutQTmp)));
        
        /**************************************************************** 
            vqmovn : values are saturated to the result width.  
            inconsistent with C instruction processing so mofidy  vqmovn to vmovn;
        *****************************************************************/
        vst1_s16(&dataOut_I[ucIndx*4],vmovn_s32(vcvtq_s32_f32(dataOutITmp)));
        vst1_s16(&dataOut_Q[ucIndx*4],vmovn_s32(vcvtq_s32_f32(dataOutQTmp)));
    }
    for(ucIndx = ucIndx *4; ucIndx < ucLen;ucIndx++)
    {
        dataOut_I[ucIndx] = dataIn_I[ucIndx]*SinParam[ucIndx] - dataIn_Q[ucIndx]*CosParam[ucIndx];
        dataOut_Q[ucIndx] = dataIn_Q[ucIndx]*SinParam[ucIndx] + dataIn_I[ucIndx]*CosParam[ucIndx];
    }
    for(ucIndx = 0;ucIndx < ucLen;ucIndx++)
    {
        printf("I:%6d-Q:%6d|",dataOut_I[ucIndx],dataOut_Q[ucIndx]);
    }
    printf("\n");
    printf("[-CPU3-]:NEON Intrinsics done \n");
    printf("[-CPU3-]:int convert to  short test \n");
    printf("[-CPU3-]:RAW Data: \n");
    for(ucIndx = 0;ucIndx <4;ucIndx++)
    {
        printf("%d ",datatest_in[ucIndx]);
    }
    printf("\n");
    printf("[-CPU3-]:C: \n");
    for(ucIndx = 0;ucIndx <4;ucIndx++)
    {
        datatest_out[ucIndx] = datatest_in[ucIndx];
        printf("%d ",datatest_out[ucIndx]);
    }
    printf("\n");
    printf("[-CPU3-]:NEON:\r\n");
    datatest_neon = vld1q_s32(&datatest_in[0]);
    vst1_s16(&datatest_out[0],vmovn_s32(datatest_neon));
    for(ucIndx = 0;ucIndx <4;ucIndx++)
    {
        printf("%d ",datatest_out[ucIndx]);
    }
    printf("\n");
    printf("[-CPU3-]:NEON saturated:\r\n");
    vst1_s16(&datatest_out[0],vqmovn_s32(datatest_neon));
    for(ucIndx = 0;ucIndx <4;ucIndx++)
    {
        printf("%d ",datatest_out[ucIndx]);
    }
    printf("\n");
    return 0;
}


