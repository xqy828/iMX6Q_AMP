
#if !defined(__CORTEX_A9_H__)
#define __CORTEX_A9_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define _arm_mrc(coproc, opcode1, Rt, CRn, CRm, opcode2)	\
    asm volatile ("mrc p" #coproc ", " #opcode1 ", %[output], c" #CRn ", c" #CRm ", " #opcode2 "\n" : [output] "=r" (Rt))
    
#define _arm_mcr(coproc, opcode1, Rt, CRn, CRm, opcode2)	\
    asm volatile ("mcr p" #coproc ", " #opcode1 ", %[input], c" #CRn ", c" #CRm ", " #opcode2 "\n" :: [input] "r" (Rt))

/* clock */

#define ARM_CLK_ROOT                                       (792000000) // 792Mhz

/*
    Cortex-A9 MPCore private memory region
    PERIPHBASE[31:13] offset
*/
#define ARM_MP_ADDR                                         (0x00A00000)
#define SCU_Registers                                       (0x0000) 
#define Interrupt_Controller_Interfaces                     (0x0100) // cpu interface
#define Global_Timer                                        (0x0200)
#define Private_Timers_and_Watchdogs                        (0x0600)
#define Interrupt_Distributor                               (0x1000)

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

/*
 * Global_Timer reg offset
 * 
 */

#define Global_Timer_Counter_low                            (0x00)                               
#define Global_Timer_Counter_high                           (0x04) 
#define Global_Timer_Control                                (0x08) 
#define Global_Timer_Counter_Interrupt_status               (0x0c) 
#define Comparator_value_low                                (0x10) 
#define Comparator_value_high                               (0x14) 
#define Auto_increment                                      (0x18) 

/*
 * Private timer reg offset  
 */

#define Prv_Timer_Load                                      (0x00)
#define Prv_Timer_Count                                     (0x04)
#define Prv_Timer_Control                                   (0x08) 
#define Prv_Timer_Intr                                      (0x0C)

/* Interrupt ID is 29 */
#define PRV_TIMER_CTRL_IRQ_ENABLE                           (1 << 2)
#define PRV_TIMER_CTRL_AUTO_RELOAD                          (1 << 1)
#define PRV_TIMER_CTRL_TIMER_ENABLE                         (1 << 0)

#define PRV_TIMER_CTR_PRESCALER_SHIFT                       (8u)
#define PRV_TIMER_INTR_EVENT                                (1 << 0)


/* Interrupt controller interfaces  cpu interface offset */
#define GICC_CTLR                                           (0x00) // CPU Interface Control Register  
#define GICC_PMR                                            (0x04) // Interrupt Priority Mask Register
#define GICC_BPR                                            (0x08) // Binary Point Register
#define GICC_IAR                                            (0x0C) // Interrupt Acknowledge Register
#define GICC_EOIR                                           (0x10) // End of Interrupt Register
#define GICC_RPR                                            (0x14) // Running Priority Register
#define GICC_HPPIR                                          (0x18) // Highest Priority Pending Interrupt Register
#define GICC_ABPR                                           (0x1c) // Aliased Binary Point Registe
#define GICC_AIAR                                           (0x20) // Aliased Interrupt Acknowledge Register
#define GICC_AEOIR                                          (0x24) // Aliased End of Interrupt Register
#define GICC_AHPPIR                                         (0x28) // Aliased Highest Priority Pending Interrupt Register

/* Interrupt distributor offset */
#define GICD_CTLR                                           (0x00) // Distributor Control Register
#define GICD_TYPER                                          (0x04) // Interrupt Controller Type Register
#define GICD_IIDR                                           (0x08) // Distributor Implementer Identification Register
#define GICD_IGROUPRn                                       (0x80) // Interrupt Group Registers
#define GICD_ISENABLERn                                     (0x100) // Interrupt Set-Enable Registers
#define GICD_ICENABLERn                                     (0x180) // Interrupt Clear-Enable Registers
#define GICD_ISPENDRn                                       (0x200) // Interrupt Set-Pending Registers
#define GICD_ICPENDRn                                       (0x280) // Interrupt Clear-Pending Registers
#define GICD_ISACTIVERn                                     (0x300) // GICv2 Interrupt Set-Active Registers
#define GICD_ICACTIVERn                                     (0x380) // Interrupt Clear-Active Registers
#define GICD_IPRIORITYRn                                    (0x400) // Interrupt Priority Registers
#define GICD_ITARGETSRn                                     (0x800) // Interrupt Processor Targets Registers    
#define GICD_ICFGRn                                         (0xC00) // Interrupt Configuration Registers
#define GICD_NSACRn                                         (0xE00) // Non-secure Access Control Registers, optional
#define GICD_SGIR                                           (0xF00) // Software Generated Interrupt Register
#define GICD_CPENDSGIRn                                     (0xF10) // SGI Clear-Pending Registers
#define GICD_SPENDSGIRn                                     (0xF20) // SGI Set-Pending Registers


#define SCU_TMR_INT_ID		                                (29)	/* SCU Private Timer interrupt */



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
