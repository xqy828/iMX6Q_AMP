
#if !defined(__CORTEX_A9_H__)
#define __CORTEX_A9_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define _arm_mrc(coproc, opcode1, Rt, CRn, CRm, opcode2)	\
    asm volatile ("mrc p" #coproc ", " #opcode1 ", %[output], c" #CRn ", c" #CRm ", " #opcode2 "\n" : [output] "=r" (Rt))
    
#define _arm_mcr(coproc, opcode1, Rt, CRn, CRm, opcode2)	\
    asm volatile ("mcr p" #coproc ", " #opcode1 ", %[input], c" #CRn ", c" #CRm ", " #opcode2 "\n" :: [input] "r" (Rt))

/*
    Cortex-A9 MPCore private memory region
    PERIPHBASE[31:13] offset
*/
#define SCU_Registers                                       (0x0000) 
#define Interrupt_Controller_Interfaces                     (0x0100)
#define Global_Timer                                        (0x0200)
#define Private_Timers_and_Watchdogs                        (0x0600)
#define Interrupt_Distributor                               (0x0700)

/*
 * SCU REG OFFSET 
*/
#define SCU_Control_Register                                (0x00)
#define SCU_Configuration_Register                          (0x04)
#define SCU_CPU_Power_Status_Register                       (0x08)
#define SCU_Invalidate_All_Registers_in_Secure_State        (0x0c)
#define Filtering_Start_Address_Register                    (0x40)
#define Filtering_End_Address_Register                      (0x44)
#define SCU_Access_Control_Register                         (0x50)
#define SCU_Non_secure_Access_Control_Register              (0x54)

void Enable_SIMD_VFP(void);
int get_cpuid(void);
unsigned int read_cpsr_reg(void);
unsigned int read_cbar_reg(void);
void disp_scu_all_regs(void);
void disable_mmu(void);
void enable_mmu(void);
void invalidate_tlb(void);
void arm_branch_target_cache_invalidate(void);
void arm_branch_prediction_enable(void);
void arm_branch_prediction_disable(void);
void scu_enable(void);
void scu_join_smp(void);
void scu_enable_maintenance_broadcast(void);
void disable_strict_align_check(void);



#endif
