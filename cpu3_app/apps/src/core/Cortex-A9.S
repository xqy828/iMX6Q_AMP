    
    .code 32
    .section ".text","ax"

    .extern  dump_regs

    .global Enable_SIMD_VFP
    .func Enable_SIMD_VFP
Enable_SIMD_VFP:
    push {lr}
	/* set NSACR, both Secure and Non-secure access are allowed to Cortex-A9 NEON MPE */
	MRC p15, 0, r0, c1, c1, 2
	ORR r0, r0, #(0x3<<10) @ set CP10 and CP11 reg can be access in Secure or Non-secure mode
	MCR p15, 0, r0, c1, c1, 2
    @bl dump_regs /* debug */
    /* Set CPACR , sets access rights for the coprocessors CP10 and CP11 */
    MRC p15, 0, r1, c1, c0, 2  /* read cpacr access control register (CPACR) into r1 */
    ORR r1, r1, #(0xf << 20)   @Privileged and User mode access CP10 and CP11
    MCR p15, 0, r1, c1, c0, 2	
    @bl dump_regs /*  debug */   
    VMRS    r1, FPEXC
    ORR r1, r1, #0x40000000
    VMSR FPEXC, r1
    @bl dump_regs /* debug */
    pop {lr}
    bx  lr
    .endfunc
    
  .global get_cpuid
  @ int get_cpuid(void)@
  .func get_cpuid
get_cpuid: 
    mrc   p15, 0, r0, c0, c0, 5
    and   r0, r0, #0xf
	bx	  lr
  .endfunc

    .global disable_mmu
    .func disable_mmu
disable_mmu:
    MRC p15, 0, r0, c1, c0, 0 @ Read System Control Register configuration data
    BIC r0, r0, #0x1
    MCR p15, 0, r0, c1, c0, 0 @ Write System Control Register configuration data
    bx lr
    .endfunc

    .global enable_mmu
    .func enable_mmu
enable_mmu:
    MRC p15, 0, r0, c1, c0, 0 @ Read System Control Register configuration data
    ORR r0, r0, #0x1 @ Bit 0 is the MMU enable
    MCR p15, 0, r0, c1, c0, 0 @ Write System Control Register configuration data
    bx lr
    .endfunc

    .global disable_l1_caches
    .func disable_l1_caches
disable_l1_caches:
    MRC p15, 0, r0, c1, c0, 0 @ Read System Control Register configuration data
    BIC r0, r0, #(0x1 << 12) @ disable I Cache
    BIC r0, r0, #(0x1 << 2) @ disable D Cache
    MCR p15, 0, r0, c1, c0, 0 @ Write System Control Register configuration data
    bx lr
    .endfunc

    .global invalidate_tlb
    .func invalidate_tlb
invalidate_tlb:
    mov r0,#0
    mcr p15,0,r0,c8,c7,0
    dsb
    bx lr
    .endfunc
    
  .global arm_branch_target_cache_invalidate
  @ void arm_branch_target_cache_invalidate(void)
  .func arm_branch_target_cache_invalidate
arm_branch_target_cache_invalidate:
  mov     r0, #0
  mcr     p15, 0, r0, c7, c5, 6                 @ BPIALL - Invalidate entire branch predictor array
  bx      lr
  .endfunc

  .global arm_branch_prediction_enable
  @ void arm_branch_prediction_enable(void)
  .func arm_branch_prediction_enable
arm_branch_prediction_enable:
  mrc     p15, 0, r0, c1, c0, 0                 @ Read SCTLR
  orr     r0, r0, #(1 << 11)                    @ Set the Z bit (bit 11)
  mcr     p15, 0,r0, c1, c0, 0                  @ Write SCTLR
  bx      lr
  .endfunc

  .global arm_branch_prediction_disable
  @ void arm_branch_prediction_disable(void)
  .func arm_branch_prediction_disable
arm_branch_prediction_disable:
  mrc     p15, 0, r0, c1, c0, 0                 @ Read SCTLR
  bic     r0, r0, #(1 << 11)                    @ Clear the Z bit (bit 11)
  mcr     p15, 0,r0, c1, c0, 0                  @ Write SCTLR
  bx      lr
  .endfunc

.global scu_enable
  @ void scu_enable(void)
  @ Enables the SCU
  .func scu_enable
scu_enable:

  mrc     p15, 4, r0, c15, c0, 0  @ Read periph base address

  ldr     r1, [r0, #0x0]          @ Read the SCU Control Register
  orr     r1, r1, #0x1            @ Set bit 0 (The Enable bit)
  str     r1, [r0, #0x0]          @ Write back modifed value

  bx      lr
  .endfunc

  .global  scu_join_smp
  @ void scu_join_smp(void)
  @ Set this CPU as participating in SMP
  .func scu_join_smp
scu_join_smp:

  @ SMP status is controlled by bit 6 of the CP15 Aux Ctrl Reg

  mrc     p15, 0, r0, c1, c0, 1   @ Read ACTLR
  orr     r0, r0, #0x040          @ Set bit 6
  mcr     p15, 0, r0, c1, c0, 1   @ Write ACTLR

  bx      lr
  .endfunc

  .global scu_enable_maintenance_broadcast
  @ void scu_enable_maintenance_broadcast(void)
  @ Enable the broadcasting of cache & TLB maintenance operations
  @ When enabled AND in SMP, broadcast all "inner sharable"
  @ cache and TLM maintenance operations to other SMP cores
  .func scu_enable_maintenance_broadcast
scu_enable_maintenance_broadcast:
  mrc     p15, 0, r0, c1, c0, 1   @ Read Aux Ctrl register
  orr     r0, r0, #0x01           @ Set the FW bit (bit 0)
  mcr     p15, 0, r0, c1, c0, 1   @ Write Aux Ctrl register

  bx      lr
  .endfunc


  .global disable_strict_align_check
  .func disable_strict_align_check
disable_strict_align_check:
  	/*Ray's note: disable strict alignment fault checking.
 	without disabling this, data abort will happen when accessing
	the BPB structure of file system since it is packed.*/
    
  	push	{r0, lr}
	
	mrc p15, 0, r0, c1, c0, 0
	bic r0, r0, #(0x1<<1) @clear A bit of SCTLR 
	mcr p15, 0, r0, c1, c0, 0

    pop {r0, pc}
  .endfunc


    .end
