#ifndef ARMV7A_REGS_H_
#define ARMV7A_REGS_H_

enum
{
  REG_R0 = 0,
# define REG_R0 REG_R0
  REG_R1 = 1,
# define REG_R1 REG_R1
  REG_R2 = 2,
# define REG_R2 REG_R2
  REG_R3 = 3,
# define REG_R3 REG_R3ss
  REG_R4 = 4,
# define REG_R4 REG_R4
  REG_R5 = 5,
# define REG_R5 REG_R5
  REG_R6 = 6,
# define REG_R6 REG_R6
  REG_R7 = 7,
# define REG_R7 REG_R7
  REG_R8 = 8,
# define REG_R8 REG_R8
  REG_R9 = 9,
# define REG_R9 REG_R9
  REG_R10 = 10,
# define REG_R10        REG_R10
  REG_R11 = 11,
# define REG_R11        REG_R11
  REG_R12 = 12,
# define REG_R12        REG_R12
  REG_R13 = 13,
# define REG_R13        REG_R13
  REG_R14 = 14,
# define REG_R14        REG_R14
  REG_R15 = 15
# define REG_R15        REG_R15
};

#define CTX_REGS_END		U(0x20)
#define WORD_SHIFT              U(2)
#define DEFINE_REG_STRUCT(name, num_regs)       \
        typedef struct name {                   \
                unsigned int ctx_regs[num_regs];    \
        }  __aligned(8) name##_t
#define CTX_REG_ALL             (CTX_REGS_END >> WORD_SHIFT)

DEFINE_REG_STRUCT(regs, CTX_REG_ALL);

typedef struct cpu_context
{
    regs_t regs_ctx;
} cpu_context_t;

#endif

typedef unsigned long elf_gregset_t[18];


