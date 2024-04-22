/*
 * Copyright (c) 2008-2012, Freescale Semiconductor, Inc.
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
 * @file asm_defines.h
 * @brief defines for startup assembly code
 *
 * @ingroup diag_util
 */

#ifndef _ASM_DEFINES_H_
#define _ASM_DEFINES_H_

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

//! @name CPSR fields
//@{
#define CPSR_N (1 << 31)    //!< Negative
#define CPSR_Z (1 << 30)    //!< Zero
#define CPSR_C (1 << 29)    //!< Carry
#define CPSR_V (1 << 28)    //!< Overflow
#define CPSR_Q (1 << 27)    //!< Saturation
#define CPSR_E (1 << 9)     //!< Endianness
#define CPSR_A (1 << 8)     //!< Async abort mask
#define CPSR_I (1 << 7)     //!< IRQ mask
#define CPSR_F (1 << 6)     //!< FIQ mask
#define CPSR_T (1 << 5)     //!< Thumb mode
#define CPSR_MODE   (0x1f)  //!< Current processor mode
//@}

//! @name Mode bits in CPSR
//@{
#define MODE_USR     0x10   //!< User mode
#define MODE_FIQ     0x11   //!< FIQ mode
#define MODE_IRQ     0x12   //!< IRQ mode
#define MODE_SVC     0x13   //!< Supervisor mode
#define MODE_ABT     0x17   //!< Abort exception mode
#define MODE_UND     0x1B   //!< Undefined instruction exception mode
#define MODE_SYS     0x1F   //!< System mode
//@}

//! @name Interrupt enable bits in CPSR
//@{
#define I_BIT        0x80    //!< When I bit is set, IRQ is disabled
#define F_BIT        0x40    //!< When F bit is set, FIQ is disabled
//@}

//! @name Stack sizes
//@{

//! @brief Size of stacks for exceptions.
#define EXCEPTION_STACK_SIZE 2048

//! @brief Supervisor mode stack size.
//!
//! This stack is much larger because most application code runs in
//! Supervisor mode.
#define SVC_STACK_SIZE 8192

//@}

//! @name Instruction macros
//@{
#define _ARM_NOP()  asm volatile ("nop\n\t")
#define _ARM_WFI()  asm volatile ("wfi\n\t")
#define _ARM_WFE()  asm volatile ("wfe\n\t")
#define _ARM_SEV()  asm volatile ("sev\n\t")
#define _ARM_DSB()  asm volatile ("dsb\n\t")
#define _ARM_ISB()  asm volatile ("isb\n\t")

#define _ARM_MRC(coproc, opcode1, Rt, CRn, CRm, opcode2)        \
    asm volatile ("mrc p" #coproc ", " #opcode1 ", %[output], c" #CRn ", c" #CRm ", " #opcode2 "\n" : [output] "=r" (Rt))

#define _ARM_MCR(coproc, opcode1, Rt, CRn, CRm, opcode2)        \
    asm volatile ("mcr p" #coproc ", " #opcode1 ", %[input], c" #CRn ", c" #CRm ", " #opcode2 "\n" :: [input] "r" (Rt))
//@}

//! @name SCTLR
//@{
#define BM_SCTLR_TE (1 << 30)  //!< Thumb exception enable.
#define BM_SCTLR_AFE (1 << 29) //!< Access flag enable.
#define BM_SCTLR_TRE (1 << 28) //!< TEX remap enable.
#define BM_SCTLR_NMFI (1 << 27)    //!< Non-maskable FIQ support.
#define BM_SCTLR_EE (1 << 25)  //!< Exception endianess.
#define BM_SCTLR_VE (1 << 24)  //!< Interrupt vectors enable.
#define BM_SCTLR_FI (1 << 21)   //!< Fast interrupt configurable enable.
#define BM_SCTLR_RR (1 << 14)  //!< Round Robin
#define BM_SCTLR_V (1 << 13)   //!< Vectors
#define BM_SCTLR_I (1 << 12)   //!< Instruction cache enable
#define BM_SCTLR_Z (1 << 11)   //!< Branch prediction enable
#define BM_SCTLR_SW (1 << 10)  //!< SWP and SWPB enable
#define BM_SCTLR_CP15BEN (1 << 5)  //!< CP15 barrier enable
#define BM_SCTLR_C (1 << 2)    //!< Data cache enable
#define BM_SCTLR_A (1 << 1)    //!< Alignment check enable
#define BM_SCTLR_M (1 << 0)    //!< MMU enable
//@}

//! @name ACTLR
//@{
#define BM_ACTLR_SMP (1 << 6)
//@}

//! @name DFSR
//@{
#define BM_DFSR_WNR (1 << 11)   //!< Write not Read bit. 0=read, 1=write.
#define BM_DFSR_FS4 (0x400)      //!< Fault status bit 4..
#define BP_DFSR_FS4 (10)        //!< Bit position for FS[4].
#define BM_DFSR_FS (0xf)      //!< Fault status bits [3:0].
//@}






#endif /*_ASM_DEFINES_H_ */
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
