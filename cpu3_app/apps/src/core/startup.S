/*
 * Copyright (c) 2010-2012, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
  
/*!
 * @file startup.s
 * @brief this file contains startup code with support for multi-core startup
 */

#include "asm_defines.h"

    // External symbol references
    .global __bss_start__
    .global __bss_end__
    .global _sys_exit
    .global _start

    .global __system_stack
    .global __irq_stack
    .global __supervisor_stack
    .global __abort_stack
    .global __fiq_stack
    .global __undef_stack

    /* Stack Pointer locations for boot code */
    .set Undef_stack,   __undef_stack
    .set FIQ_stack,     __fiq_stack
    .set Abort_stack,   __abort_stack
    .set SUP_stack,     __supervisor_stack
    .set IRQ_stack,     __irq_stack
    .set SYS_stack,     __system_stack

    .code 32
    .section ".boot","ax"
    
    .global _boot
    .func _boot 
    .fnstart
_boot:
    @ save r0 for cores 1-3, r0 arg field passed by ROM
    @ r0 is a function pointer for secondary cpus
    mov 	r4, r0
    
    @ clear some registers
    mov     r11,#0
    mov     r12,#0
    mov     lr,#0
 
    @ set SVC MODE 
    @mrs r2, cpsr
    @bic r2, r2, #0x1f
    @orr r2, r2, #0xd3
    @msr cpsr, r2

    mrc p15, 0 ,r1, c1, c0, 0
    bic r1, r1, #0x00002000    @clear bit 13 
    mcr p15, 0, r1, c1, c0, 0
    ldr r1,=_start
    mcr p15, 0, r1, c12, c0, 0 @set VBAR

    mrs	r0, cpsr                        /* get the current PSR */
    mvn	r1, #0x1f                       /* set up the irq stack pointer */
    and	r2, r1, r0
    orr	r2, r2, #0x12                   /* IRQ mode */
    msr	cpsr, r2
    ldr	r13,=IRQ_stack                  /* IRQ stack pointer */
    bic r2, r2, #(0x1 << 9)             /* Set EE bit to little-endian */
    msr spsr_fsxc,r2

    mrs	r0, cpsr                        /* get the current PSR */
    mvn	r1, #0x1f                       /* set up the Abort  stack pointer */
    and	r2, r1, r0
    orr	r2, r2, #0x17                   /* Abort mode */
    msr	cpsr, r2
    ldr	r13,=Abort_stack                /* Abort stack pointer */
    bic r2, r2, #(0x1 << 9)             /* Set EE bit to little-endian */
    msr spsr_fsxc,r2

    mrs	r0, cpsr                        /* get the current PSR */
    mvn	r1, #0x1f                       /* set up the FIQ stack pointer */
    and	r2, r1, r0
    orr	r2, r2, #0x11                   /* FIQ mode */
    msr	cpsr, r2
    ldr	r13,=FIQ_stack                  /* FIQ stack pointer */
    bic r2, r2, #(0x1 << 9)             /* Set EE bit to little-endian */
    msr spsr_fsxc,r2

    mrs	r0, cpsr                        /* get the current PSR */
    mvn	r1, #0x1f                       /* set up the Undefine stack pointer */
    and	r2, r1, r0
    orr	r2, r2, #0x1b                   /* Undefine mode */
    msr	cpsr, r2
    ldr	r13,=Undef_stack                /* Undefine stack pointer */
    bic r2, r2, #(0x1 << 9)             /* Set EE bit to little-endian */
    msr spsr_fsxc,r2

    mrs	r0, cpsr                        /* get the current PSR */
    mvn	r1, #0x1f                       /* set up the system stack pointer */
    and	r2, r1, r0
    orr	r2, r2, #0x1F                   /* SYS mode */
    msr	cpsr, r2
    ldr	r13,=SYS_stack                  /* SYS stack pointer */
    bic r2, r2, #(0x1 << 9)             /* Set EE bit to little-endian */
    msr spsr_fsxc,r2

    mrs	r0, cpsr                        /* get the current PSR */
    mvn	r1, #0x1f                       /* set up the supervisor stack pointer */
    and	r2, r1, r0
    orr	r2, r2, #0x13                   /* supervisor mode */
    msr	cpsr, r2
    ldr	r13,=SUP_stack                  /* Supervisor stack pointer */
    bic r2, r2, #(0x1 << 9)             /* Set EE bit to little-endian */
    msr spsr_fsxc,r2

    /* init .bss */
    /* clear the .bss section (zero init) */
    ldr     r1,=__bss_start__
    ldr     r2,=__bss_end__
    mov     r3,#0
1:  cmp     r1,r2
    stmltia r1!,{r3}
    blt     1b

    @ take care of C++ static initialization. this won't cause any harm for plain C apps.
    ldr     r12,=__libc_init_array
    blx     r12
    
    bx      r4     @ for secondary cpus, jump to argument function pointer passed in by ROM
    @ control should not return from the secondary cpu entry point
    b       startup_exit
    
    startup_exit:
    mov     r0,#0
    ldr     r12,=_sys_exit
    bx      r12
    
    .cantunwind
    .fnend
    .endfunc    @ _start()

    /* local label for the stack fill pattern */
.Ldeadfeed:
    .word   0xdeadfeed


    .end

