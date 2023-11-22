#include "asm_defines.h"

    .code 32
    
    .global _boot
    .global data_abort_handler
    .global prefetch_abort_handler

    .extern  dump_regs
    .section .vectors,"ax"

    .global _start

_start:
    ldr     pc, .reset
    ldr     pc, .undefined_instruction
    ldr     pc, .swi_interrupt
    ldr     pc, .prefetch_abort
    ldr     pc, .data_abort
    ldr     pc, .reserved_not_used /* not used vector */
    ldr     pc, .irq
    ldr     pc, .fiq

.reset:
    .word     _boot
.undefined_instruction:
    .word     undefined_handler
.swi_interrupt:
    .word     swi_handler
.prefetch_abort:
    .word     prefetch_abort_handler
.data_abort:
    .word     data_abort_handler
.reserved_not_used:
    .word     exception_handler
.irq:
    .word     irq_handler
.fiq:
    .word     exception_handler

.pad:
    .word   0x12345678

    .text


    .global irq_handler
    .func irq_handler
irq_handler:
    stmdb   sp!,{r0-r3,r12,lr}      /* state save from compiled code*/
    /* ARM VFP & NEON */
    vpush {d0-d7}
    vpush {d16-d31}
    vmrs r1, FPSCR
    push {r1}
    vmrs r1, FPEXC
    push {r1}
    
    @bl  irq_interrupt            /* to do */
     /* ARM VFP & NEON */
	pop     {r1}
    vmsr    FPEXC, r1
    pop     {r1}
    vmsr    FPSCR, r1
    vpop    {d16-d31}
    vpop    {d0-d7}

    ldmia   sp!,{r0-r3,r12,lr}      /* state restore from compiled code */

    subs    pc, lr, #4              /* adjust return */
    .endfunc

    .global undefined_handler
    .func undefined_handler
undefined_handler:                  /* Undefined handler */
    stmdb   sp!,{r0-r3,r12,lr}      /* state save from compiled code */
    ldmia   sp!,{r0-r3,r12,lr}      /* state restore from compiled code */
    b   _boot
    movs    pc, lr
    .endfunc

    .global exception_handler
    .func exception_handler
exception_handler:
1:
        b       1b
    .endfunc
    
    .global swi_handler
    .func swi_handler
swi_handler:
    stmdb   sp!,{r0-r3,r12,lr}      /* state save from compiled code */

    tst r0, #0x20                   /* check the T bit */
    ldrneh  r0, [lr,#-2]            /* Thumb mode */
    bicne   r0, r0, #0xff00         /* Thumb mode */
    ldreq   r0, [lr,#-4]            /* ARM mode */
    biceq   r0, r0, #0xff000000     /* ARM mode */

    @bl  swi_interrupt              /* to do */

    ldmia   sp!,{r0-r3,r12,lr}      /* state restore from compiled code */
    movs    pc, lr                  /*return to the next instruction after the SWI instruction */

    .endfunc

    .global  common_abort_handler
    .func  common_abort_handler
common_abort_handler: 
	push    {r0-r12}	        // push the general registers to this exception's stack
	mrs     r7, cpsr
	mrs     r6, spsr
	mov     r5, sp
	msr     cpsr, r6            // go to the mode before this exception so we can get its sp and pc
	dsb
	stmfd   r5!, {sp, lr}       // push sp/lr to this exception's stack
	msr     cpsr, r7            // switch back to abort mode
	dsb
	mov     sp, r5              // restore our stack pointer
	push    {r6, lr}            // r6 is exception's cpsr, lr is the exception's pc
	
	// save coprocessor abort info registers
	mrc     p15, 0, r4, c5, c0, 0   // read DFSR
	mrc     p15, 0, r3, c6, c0, 0   // read DFAR
	mrc     p15, 0, r2, c5, c0, 1   // read IFSR
	mrc     p15, 0, r1, c6, c0, 2   // read IFAR
	push    {r1-r4}             // save DFSR, DFAR, IFSR, IFAR to stack
	
	ldr     r1,=.abort_type
	ldr     r0,[r1]     // put abort type in r0
	mov     r1, sp      // put address to saved registers into r1
	bl      dump_regs   // call a C function to print the register values
	
	// clean up
	pop     {r0-r12}
	add     sp, sp, #16 // adjust the sp pointer, just like 'pop {lr, r6}'

    // Loop forever.
1:
	b       1b
	
	sub     pc, lr, #4 // abort handler's standard return, although never reach here.
    .endfunc

    .global data_abort_handler
    .func data_abort_handler
data_abort_handler:
    push    {r0-r1}
    ldr     r1,=.abort_type
    mov     r0,#0
    str     r0,[r1]
    pop     {r0-r1}
    b       common_abort_handler
    .endfunc

    .global prefetch_abort_handler
    .func prefetch_abort_handler
prefetch_abort_handler:
    push    {r0-r1}
    ldr     r1,=.abort_type
    mov     r0,#1
    str     r0,[r1]
    pop     {r0-r1}
    b       common_abort_handler
    .endfunc

    .data

.abort_type:
    .word  0

    .end


