    
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
    bx      lr
    .endfunc
    

    .end
