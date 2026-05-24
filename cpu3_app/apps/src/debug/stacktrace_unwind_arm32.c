// SPDX-License-Identifier: BSD-2-Clause
 /*******************************************************************//*
 * This file is derived from the OP-TEE project with modifications.
 * /optee_os-4.3.0/core/arch/arm/kernel/unwind_arm32.c
 * /optee_os-4.3.0/lib/libunw/unwind_arm32.c
 ********************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

/* The register names */
#define	FP	11
#define	SP	13
#define	LR	14
#define	PC	15

/*
 * Definitions for the instruction interpreter.
 *
 * The ARM EABI specifies how to perform the frame unwinding in the
 * Exception Handling ABI for the ARM Architecture document. To perform
 * the unwind we need to know the initial frame pointer, stack pointer,
 * link register and program counter. We then find the entry within the
 * index table that points to the function the program counter is within.
 * This gives us either a list of three instructions to process, a 31-bit
 * relative offset to a table of instructions, or a value telling us
 * we can't unwind any further.
 *
 * When we have the instructions to process we need to decode them
 * following table 4 in section 9.3. This describes a collection of bit
 * patterns to encode that steps to take to update the stack pointer and
 * link register to the correct values at the start of the function.
 */

/* A special case when we are unable to unwind past this function */
#define	EXIDX_CANTUNWIND	1

/*
 * Entry types.
 * These are the only entry types that have been seen in the kernel.
 */
#define	ENTRY_MASK	0xff000000
#define	ENTRY_ARM_SU16	0x80000000
#define	ENTRY_ARM_LU16	0x81000000

/* Instruction masks. */
#define	INSN_VSP_MASK		0xc0
#define	INSN_VSP_SIZE_MASK	0x3f
#define	INSN_STD_MASK		0xf0
#define	INSN_STD_DATA_MASK	0x0f
#define	INSN_POP_TYPE_MASK	0x08
#define	INSN_POP_COUNT_MASK	0x07
#define	INSN_VSP_LARGE_INC_MASK	0xff

/* Instruction definitions */
#define	INSN_VSP_INC		0x00
#define	INSN_VSP_DEC		0x40
#define	INSN_POP_MASKED		0x80
#define	INSN_VSP_REG		0x90
#define	INSN_POP_COUNT		0xa0
#define	INSN_FINISH		0xb0
#define	INSN_POP_REGS		0xb1
#define	INSN_VSP_LARGE_INC	0xb2
#define SHIFT_U32(v, shift)	((unsigned int)(v) << (shift))
#define BIT32(nr)       ((unsigned int)(1) << (nr))
#ifndef  __always_inline 
#define __always_inline     inline __attribute__((__always_inline__))
#endif

#ifndef __noprof
#define __noprof	__attribute__((no_instrument_function))
#endif

#ifndef __unused 
#define __unused __attribute__((unused))
#endif

extern char __exidx_start;
extern char __exidx_end;
extern char _supervisor_stack_end;
extern char __supervisor_stack;

/* An item in the exception index table */
struct unwind_idx {
	unsigned int offset;
	unsigned int insn;
};

/* The state of the unwind process (32-bit mode) */
struct unwind_state_arm32 {
	unsigned int registers[16];
	unsigned int start_pc;
	unsigned long  insn;
	unsigned int entries;
	unsigned int byte;
	unsigned short update_mask;
};

static __always_inline __noprof unsigned int read_sp(void)
{
	unsigned int val;
	asm volatile ("mov %0, sp" : "=r" (val));
	return val;
}

static __always_inline __noprof unsigned int read_lr(void)
{
	unsigned int val;
	asm volatile ("mov %0, lr" : "=r" (val));
	return val;
}

static __always_inline __noprof unsigned int read_fp(void)
{
	unsigned int val;
	asm volatile ("mov %0, fp" : "=r" (val));
	return val;
}

static __always_inline __noprof unsigned int read_r7(void)
{
	unsigned int val;
	asm volatile ("mov %0, r7" : "=r" (val));
	return val;
}

/*****************************************************************************//*
*    PROVIDE_HIDDEN(__exidx_start = .);
*    .ARM.exidx :
*    {
*        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
*    } > DDR
*    PROVIDE_HIDDEN(__exidx_end = .);
********************************************************************************/
bool find_exidx(unsigned long int *idx_start, unsigned long int *idx_end)
{
	*idx_start = (unsigned long int)__exidx_start;
	*idx_end = (unsigned long int)__exidx_end;
	return true;
}

static bool copy_in(void *dst, const void *src, size_t n)
{
	memcpy(dst, src, n);
	return true;
}

static inline void __noprof get_stack_hard_limits(unsigned long int *bottom ,unsigned long int *top)
{	
	/* The CPU finally runs in Supervisor mode. */
	unsigned long stack_bottom = (unsigned long)_supervisor_stack_end;
	unsigned long stack_top = (unsigned long)__supervisor_stack;
	size_t stack_sz = stack_bottom - stack_top;
	*top = stack_top;
	*bottom = stack_bottom;	
}

/* Expand a 31-bit signed value to a 32-bit signed value */
static int expand_prel31(unsigned int prel31)
{
	return prel31 | SHIFT_U32(prel31 & BIT32(30), 1);
}

/*
 * Perform a binary search of the index table to find the function
 * with the largest address that does not exceed addr.
 */
static struct unwind_idx *find_index(unsigned int addr)
{
	unsigned long  idx_start = 0;
	unsigned long  idx_end = 0;
	unsigned int min = 0;
	unsigned int mid = 0;
	unsigned int max = 0;
	struct unwind_idx *start = NULL;
	struct unwind_idx *item = NULL;
	int prel31_addr = 0;
	unsigned long  func_addr = 0;

	if (!find_exidx(&idx_start, &idx_end))
		return NULL;

	start = (struct unwind_idx *)idx_start;

	min = 0;
	max = (idx_end - idx_start) / sizeof(struct unwind_idx);

	while (min != max) {
		mid = min + (max - min + 1) / 2;

		item = &start[mid];

		prel31_addr = expand_prel31(item->offset);
		func_addr = (unsigned long )&item->offset + prel31_addr;

		if (func_addr <= addr)
			min = mid;
		else
			max = mid - 1;
	}

	return &start[min];
}

/* Reads the next byte from the instruction list */
static bool unwind_exec_read_byte(struct unwind_state_arm32 *state,
				  unsigned int *ret_insn)
{
	unsigned int insn;

	if (!copy_in(&insn, (void *)state->insn, sizeof(insn)))
		return false;

	/* Read the unwind instruction */
	*ret_insn = (insn >> (state->byte * 8)) & 0xff;

	/* Update the location of the next instruction */
	if (state->byte == 0) {
		state->byte = 3;
		state->insn += sizeof(unsigned int);
		state->entries--;
	} else {
		state->byte--;
	}

	return true;
}

static bool pop_vsp(unsigned int *reg, unsigned long  *vsp, unsigned long  stack,
		    size_t stack_size)
{
	if (*vsp < stack)
		return false;
	if (*vsp + sizeof(*reg) > stack + stack_size)
		return false;

	if (!copy_in(reg, (void *)*vsp, sizeof(*reg)))
		return false;
	(*vsp) += sizeof(*reg);
	return true;
}

/* Executes the next instruction on the list */
static bool unwind_exec_insn(struct unwind_state_arm32 *state, unsigned long  stack,
			     size_t stack_size)
{
	unsigned int insn;
	unsigned long  vsp = state->registers[SP];
	int update_vsp = 0;

	/* Read the next instruction */
	if (!unwind_exec_read_byte(state, &insn))
		return false;

	if ((insn & INSN_VSP_MASK) == INSN_VSP_INC) {
		state->registers[SP] += ((insn & INSN_VSP_SIZE_MASK) << 2) + 4;

	} else if ((insn & INSN_VSP_MASK) == INSN_VSP_DEC) {
		state->registers[SP] -= ((insn & INSN_VSP_SIZE_MASK) << 2) + 4;

	} else if ((insn & INSN_STD_MASK) == INSN_POP_MASKED) {
		unsigned int mask;
		unsigned int reg;

		/* Load the mask */
		if (!unwind_exec_read_byte(state, &mask))
			return false;
		mask |= (insn & INSN_STD_DATA_MASK) << 8;

		/* We have a refuse to unwind instruction */
		if (mask == 0)
			return false;

		/* Update SP */
		update_vsp = 1;

		/* Load the registers */
		for (reg = 4; mask && reg < 16; mask >>= 1, reg++) {
			if (mask & 1) {
				if (!pop_vsp(&state->registers[reg], &vsp,
					     stack, stack_size))
					return false;
				state->update_mask |= 1 << reg;

				/* If we have updated SP kep its value */
				if (reg == SP)
					update_vsp = 0;
			}
		}

	} else if ((insn & INSN_STD_MASK) == INSN_VSP_REG &&
		   ((insn & INSN_STD_DATA_MASK) != 13) &&
		   ((insn & INSN_STD_DATA_MASK) != 15)) {
		/* sp = register */
		state->registers[SP] =
		    state->registers[insn & INSN_STD_DATA_MASK];

	} else if ((insn & INSN_STD_MASK) == INSN_POP_COUNT) {
		unsigned int count, reg;

		/* Read how many registers to load */
		count = insn & INSN_POP_COUNT_MASK;

		/* Update sp */
		update_vsp = 1;

		/* Pop the registers */
		for (reg = 4; reg <= 4 + count; reg++) {
			if (!pop_vsp(&state->registers[reg], &vsp,
				     stack, stack_size))
				return false;
			state->update_mask |= 1 << reg;
		}

		/* Check if we are in the pop r14 version */
		if ((insn & INSN_POP_TYPE_MASK) != 0) {
			if (!pop_vsp(&state->registers[14], &vsp,
				     stack, stack_size))
				return false;
		}

	} else if (insn == INSN_FINISH) {
		/* Stop processing */
		state->entries = 0;

	} else if (insn == INSN_POP_REGS) {
		unsigned int mask;
		unsigned int reg;

		if (!unwind_exec_read_byte(state, &mask))
			return false;
		if (mask == 0 || (mask & 0xf0) != 0)
			return false;

		/* Update SP */
		update_vsp = 1;

		/* Load the registers */
		for (reg = 0; mask && reg < 4; mask >>= 1, reg++) {
			if (mask & 1) {
				if (!pop_vsp(&state->registers[reg], &vsp,
					     stack, stack_size))
					return false;
				state->update_mask |= 1 << reg;
			}
		}

	} else if ((insn & INSN_VSP_LARGE_INC_MASK) == INSN_VSP_LARGE_INC) {
		unsigned int uleb128;

		/* Read the increment value */
		if (!unwind_exec_read_byte(state, &uleb128))
			return false;

		state->registers[SP] += 0x204 + (uleb128 << 2);

	} else {
		/* We hit a new instruction that needs to be implemented */
		printf("Unhandled instruction %.2x", insn);
		return false;
	}

	if (update_vsp)
		state->registers[SP] = vsp;

	return true;
}

/* Performs the unwind of a function */
static bool unwind_tab(struct unwind_state_arm32 *state, unsigned long  stack,
		       size_t stack_size)
{
	unsigned int entry;
	unsigned int insn;

	/* Set PC to a known value */
	state->registers[PC] = 0;

	if (!copy_in(&insn, (void *)state->insn, sizeof(insn))) {
		printf("Bad insn addr %p", (void *)state->insn);
		return true;
	}

	/* Read the personality */
	entry = insn & ENTRY_MASK;

	if (entry == ENTRY_ARM_SU16) {
		state->byte = 2;
		state->entries = 1;
	} else if (entry == ENTRY_ARM_LU16) {
		state->byte = 1;
		state->entries = ((insn >> 16) & 0xFF) + 1;
	} else {
		printf("Unknown entry: %x", entry);
		return true;
	}

	while (state->entries > 0) {
		if (!unwind_exec_insn(state, stack, stack_size))
			return true;
	}

	/*
	 * The program counter was not updated, load it from the link register.
	 */
	if (state->registers[PC] == 0) {
		state->registers[PC] = state->registers[LR];

		/*
		 * If the program counter changed, flag it in the update mask.
		 */
		if (state->start_pc != state->registers[PC])
			state->update_mask |= 1 << PC;
	}

	return false;
}

bool unwind_stack_arm32(struct unwind_state_arm32 *state,
			unsigned long  stack, size_t stack_size)
{
	struct unwind_idx *index;
	bool finished;

	/* Reset the mask of updated registers */
	state->update_mask = 0;

	/* The pc value is correct and will be overwritten, save it */
	state->start_pc = state->registers[PC];

	/*
	 * Find the item to run. Subtract 2 from PC to make sure that we're
	 * still inside the calling function in case a __no_return function
	 * (typically panic()) is called unconditionally and may cause LR and
	 * thus this PC to point into the next and entirely unrelated function.
	 */
	index = find_index(state->start_pc - 2);
	if (!index)
		return false;

	finished = false;
	if (index->insn != EXIDX_CANTUNWIND) {
		if (index->insn & (1U << 31)) {
			/* The data is within the instruction */
			state->insn = (unsigned long )&index->insn;
		} else {
			/* A prel31 offset to the unwind table */
			state->insn = (unsigned long )&index->insn +
				      expand_prel31(index->insn);
		}

		/* Run the unwind function */
		finished = unwind_tab(state, stack, stack_size);
	}

	/* This is the top of the stack, finish */
	if (index->insn == EXIDX_CANTUNWIND)
		finished = true;

	return !finished;
}

void print_stack_arm32(struct unwind_state_arm32 *state,
		       unsigned long  stack, size_t stack_size)
{
	unsigned long int pc = 0, lr = 0;
	pc = state->registers[PC] - 4;//dump_stack;
	printf("\nCall trace:\n");
	printf("  PC:	[< %08lx >]\n", pc);
	printf("  LR:	[< %08lx >]\n", lr);
	printf("\nStack:\n");
	do {
		pc = (unsigned long int)state->registers[PC];
		printf("	[< %08lx >]\n", pc);
	} while (unwind_stack_arm32(state, stack, stack_size));
}

void dump_stack(void)
{
	struct unwind_state_arm32 state = { };
	unsigned long int stack_bottom = 0;
	unsigned long int stack_top = 0;
	int i = 0;	
	/* Don't use memset(), which updates LR ! */
	for (i = 0; i < 16; i++)
	{
		state.registers[i] = 0;
	}
	state.update_mask = 0;
	state.start_pc = 0;
	state.entries = 0;
	state.insn = 0;
	state.byte = 0;

	/* r7: Thumb-style frame pointer */
	state.registers[7] = read_r7();
	/* r11: ARM-style frame pointer */
	state.registers[FP] = read_fp();
	state.registers[SP] = read_sp();
	state.registers[LR] = read_lr();

	/*
	 * Add 4 to make sure that we have an address well inside this function.
	 * This is needed because we're subtracting 2 from PC when calling
	 * find_index() above. See a comment there for more details.
	 */
	state.registers[PC] = (unsigned int)dump_stack + 4;
	get_stack_hard_limits(&stack_bottom, &stack_top);
	print_stack_arm32(&state, stack_top, stack_bottom - stack_top);
}


