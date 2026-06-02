// SPDX-License-Identifier: BSD-2-Clause
 /*******************************************************************//*
 * This file is derived from the OP-TEE project with modifications.
 * /optee_os-4.3.0/core/arch/arm/kernel/unwind_arm32.c
 * /optee_os-4.3.0/lib/libunw/unwind_arm32.c
 ********************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "debug.h"
#include "public.h"
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

extern char __exidx_start[];
extern char __exidx_end[];
extern char __supervisor_stack_top[];
extern char __supervisor_stack_bottom[];

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
	*idx_start = (unsigned long int)&__exidx_start;
	*idx_end = (unsigned long int)&__exidx_end;
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
	unsigned long int stack_bottom = (unsigned long int)&__supervisor_stack_bottom;
	unsigned long int stack_top = (unsigned long int)&__supervisor_stack_top;
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
		disp("Unhandled instruction %.2x", insn);
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
		disp("Bad insn addr %p\n", (void *)state->insn);
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
		disp("Unknown entry: %x\n", entry);
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
	lr = state->registers[LR];
	disp("\n");
	disp("Call trace:\n");
	disp("  PC:	[< %08lx >]\n", pc);
	disp("  LR:	[< %08lx >]\n", lr);
	disp("\n");
	disp("Stack:\n");
#if 1
	do {
		pc = (unsigned long int)state->registers[PC];
		disp("	[< %08lx >]\n", pc);
	} while (unwind_stack_arm32(state, stack, stack_size));
#endif
    disp("\nCopy info from \"Call trace...\" to a file(eg. dump.txt)\n"
        "and run command in your project: "
        "./scripts/stacktrace.sh dump.txt \n");
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
	disp("Stack:0x%lx-0x%lx\n",stack_top,stack_bottom);
	print_stack_arm32(&state, stack_top, stack_bottom - stack_top);
}


/**************************************************************************//*
* dummp stack function test  
**************************************************************************/
/* ========== 用于增加复杂性的自定义结构体 ========== */
typedef struct {
    int id;
    double value;
    char name[32];
} complex_t;

/* ========== 递归函数（深度可控） ========== */
/**
 * recursive_func - 递归调用，并记录深度
 * @depth: 当前剩余递归深度
 * @max_depth: 初始最大深度（用于局部变量）
 *
 * 当 depth == 0 时停止递归，并调用 dump_stack 打印完整调用栈。
 * 否则继续递归，同时调用其他函数增加栈帧混合。
 */
void recursive_func(int depth, int max_depth)
{
    /* 大量的局部变量，占用栈空间，测试回溯能否越过这些数据 */
    volatile int local_arr[64];
    volatile double pi = 3.14159265358979;
    volatile char msg[128] = "Inside recursive_func";
    volatile complex_t st = { .id = depth, .value = depth * 1.5, .name = "recursive" };
    volatile unsigned long marker = (unsigned long)&marker; // 栈上地址标记
	UNUSED_PARA(st);
    UNUSED_PARA(msg);
	UNUSED_PARA(pi);
	for (int i = 0; i < 64; ++i) local_arr[i] = depth + i;

    if (depth == 0) {
        disp("\n");
		disp(">>> Reached base case of recursion (depth = 0) <<<\n");
        disp(">>> Current stack should contain main -> func1 -> ... -> func6 -> recursive_func x %d <<<\n", max_depth);
        dump_stack();
        return;
    }

    /* 递归之前混合一次间接调用，增加栈帧多样化 */
    void (*func_ptr)(int, int) = recursive_func;
    disp("recursive_func: depth=%d, calling recursively...\n", depth);
    func_ptr(depth - 1, max_depth);
}

/* ========== 普通嵌套函数（第6层） ========== */
void func6(int level, double data, const char *tag)
{
    volatile int local = level * 100;
    volatile double temp = data * 2.0;
    volatile char buffer[64];
    snprintf((char*)buffer, sizeof(buffer), "func6 level=%d", level);

    disp("%s called, local=%d, temp=%.2f\n", buffer, local, temp);

    /* 启动递归，深度为 3（可根据需要修改） */
    recursive_func(3, 3);
}

/* 第5层 */
void func5(int level, double data)
{
    volatile int arr[32];
    volatile complex_t st = { level, data, "func5" };
	UNUSED_PARA(st);
    for (int i = 0; i < 32; ++i) arr[i] = level + i;

    disp("func5 level=%d, data=%.2f\n", level, data);
    func6(level + 1, data * 1.5, "from_func5");
}

/* 第4层 */
void func4(int level, double data, const char *msg)
{
    volatile char local_str[128];
    volatile int magic = 0xDEADBEEF;
    snprintf((char*)local_str, sizeof(local_str), "%s at level %d", msg, level);
    disp("func4: %s, magic=0x%X\n", local_str, magic);
    func5(level + 1, data * 1.2);
}

/* 第3层 */
void func3(int level, double data)
{
    volatile double d = data;
    volatile int counter = 0;
    disp("func3 level=%d\n", level);
    /* 简单的循环，增加栈上临时变量 */
    for (int i = 0; i < 5; ++i) {
        counter += i;
        d *= 1.01;
    }
    func4(level + 1, d, "Hello from func3");
}

/* 第2层 */
void func2(int level, const char *prefix)
{
    volatile char combined[256];
    snprintf((char*)combined, sizeof(combined), "%s[level=%d]", prefix, level);
    disp("func2: %s\n", combined);
    func3(level + 1, (double)level * 3.14);
}

/* 第1层 */
void func1(int start_level)
{
    volatile int my_level = start_level;
    volatile double pi = 3.1415926;
	UNUSED_PARA(pi);
    disp("func1 start_level=%d\n", my_level);
    func2(my_level + 1, "->func1");
}

/* ========== 间接调用（函数指针）增加复杂度 ========== */
void wrapper_func(void (*fn)(int), int arg)
{
	disp("\n");
    disp("--- Enter wrapper_func, about to call function pointer ---\n");
    fn(arg);
    disp("--- wrapper_func finished ---\n");
}

void Test_dump_stack(void)
{
    disp("=== Complex Nesting Test for dump_stack ===\n");
    disp("Call chain: main -> wrapper -> func1 -> func2 -> func3 -> func4 -> func5 -> func6 -> recursive_func x3\n");
    disp("After reaching base case of recursion, dump_stack() will be called.\n");

    /* 通过函数指针调用 func1，增加一层间接性 */
    void (*entry)(int) = func1;
    wrapper_func(entry, 1);
	disp("\n");
    disp("=== End of test ===\n");
}

/**************************************************************************//*
* end test  
**************************************************************************/


