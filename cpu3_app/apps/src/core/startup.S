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
    .global top_of_stacks
    .global __bss_start__
    .global __bss_end__
    .global _sys_exit
    .global _start
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
    
    ldr		r0, =top_of_stacks  @ symbol defined in linker file
    mov		r1, #EXCEPTION_STACK_SIZE

    @ get cpu id, and subtract the offset from the stacks base address
    mrc     p15,0,r2,c0,c0,5  @ read multiprocessor affinity register
    and     r2, r2, #3        @ mask off, leaving CPU ID field
    mov     r5, r2		      @ save cpu id for later
    
    mul     r3, r2, r1
    sub     r0, r0, r3
  
    mov     r1, r1, lsl #2
           
    @ set stacks for all other modes 
    msr     CPSR_c, #MODE_FIQ | I_BIT | F_BIT
    mov     sp, r0
    sub     r0, r0, r1 
    
    msr     CPSR_c, #MODE_IRQ | I_BIT | F_BIT
    mov     sp, r0
    sub     r0, r0, r1 
    
    msr     CPSR_c, #MODE_ABT | I_BIT | F_BIT
    mov     sp, r0
    sub     r0, r0, r1 
    
    msr     CPSR_c, #MODE_UND | I_BIT | F_BIT
    mov     sp, r0
    sub     r0, r0, r1 
    
    msr     CPSR_c, #MODE_SYS | I_BIT | F_BIT
    mov     sp, r0
    sub     r0, r0, r1 
    
    @ Set SVC mode stack with interrupts disabled 
    msr     CPSR_c, #MODE_SVC | I_BIT | F_BIT
    mov     sp, r0
    sub     r0, r0, r1
   
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

