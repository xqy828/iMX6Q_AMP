#include "public.h"
#include "cortex_a9.h"

unsigned int read_cpsr_reg(void)
{
    unsigned int cpsr_reg = 0;
    asm volatile(
        "mrs %0, cpsr\n"
        : "=r" (cpsr_reg)
        :
        :
                );
	return cpsr_reg;
}

unsigned int read_cbar_reg(void)
{
    unsigned int cbar_reg = 0;
    _arm_mrc(15, 4, cbar_reg, 15, 0, 0);
    return cbar_reg;
}

void disp_scu_all_regs(void)
{
    unsigned int cbar_reg = 0;
    unsigned int scu_reg = 0;
    cbar_reg = read_cbar_reg();
    
    scu_reg = *(unsigned int *)(cbar_reg + SCU_Registers + SCU_Control_Register);
    disp("SCU_Control_Register = 0x%08x\n",scu_reg);
    scu_reg = *(unsigned int *)(cbar_reg + SCU_Registers + SCU_Configuration_Register);
    disp("SCU_Configuration_Register = 0x%08x\n",scu_reg);
    scu_reg = *(unsigned int *)(cbar_reg + SCU_Registers + SCU_CPU_Power_Status_Register);
    disp("SCU_CPU_Power_Status_Register = 0x%08x\n",scu_reg);
    scu_reg = *(unsigned int *)(cbar_reg + SCU_Registers + SCU_Invalidate_All_Registers_in_Secure_State);
    disp("SCU_Invalidate_All_Registers_in_Secure_State = 0x%08x\n",scu_reg);
    scu_reg = *(unsigned int *)(cbar_reg + SCU_Registers + Filtering_Start_Address_Register);
    disp("Filtering_Start_Address_Register = 0x%08x\n",scu_reg);
    scu_reg = *(unsigned int *)(cbar_reg + SCU_Registers + Filtering_End_Address_Register);
    disp("Filtering_End_Address_Register = 0x%08x\n",scu_reg);
    scu_reg = *(unsigned int *)(cbar_reg + SCU_Registers + SCU_Access_Control_Register);
    disp("SCU_Access_Control_Register = 0x%08x\n",scu_reg);
    scu_reg = *(unsigned int *)(cbar_reg + SCU_Registers + SCU_Non_secure_Access_Control_Register);
    disp("SCU_Non-secure_Access_Control_Register = 0x%08x\n",scu_reg);
}




