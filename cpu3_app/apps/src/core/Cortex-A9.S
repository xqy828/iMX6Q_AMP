    
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





    .end
