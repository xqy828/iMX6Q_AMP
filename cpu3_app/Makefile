WORK_DIR := $(shell pwd)
OUTPUT := output
HOST_ARCH :=$(shell arch)
APP_SRC := apps/src

THREADX_DIR := threadx-6.4.0_rel
THREADX_ARCH := cortex_a9
THREADX_TOOLCHAIN:= gnu

CC_FLAGS += -Wall -Wno-unused-but-set-variable -Wno-format
CC_FLAGS += -O0 -g -mno-unaligned-access -fdce 
CC_FLAGS += -march=armv7-a -mtune=cortex-a9 -mapcs
CC_FLAGS += -fno-common -ffreestanding -fno-builtin -static 
CC_FLAGS += -mfpu=neon-vfpv3 -ftree-vectorize -fno-math-errno -funsafe-math-optimizations -fno-signed-zeros -mfloat-abi=softfp
CC_FLAGS += -marm
C99_FLAGS += -std=gnu99 -fgnu89-inline

CFLAGS += $(CC_FLAGS)
CFLAGS += $(C99_FLAGS)

CXXFLAGS += -fno-exceptions -fno-rtti
CXXFLAGS += $(CFLAGS)


PROG_NAME = $(OUTPUT)/cpu3_app
ELF_NAME = $(PROG_NAME).elf
MAP_NAME = $(PROG_NAME).map
DIS_NAME = $(PROG_NAME).dis
BIN_NAME = $(PROG_NAME).bin
THREADX_LIB = $(OUTPUT)/tx.a

OBJ_DIR = $(OUTPUT)/obj

CROSS_COMPILE =arm-none-eabi-
ifeq ($(HOST_ARCH),x86_64)
	CROSS_COMPILE_PATH=/opt/arm-gnu-toolchain-13.2.Rel1-x86_64-arm-none-eabi/bin/
else ifeq ($(HOST_ARCH),aarch64)
	CROSS_COMPILE_PATH=/opt/arm-gnu-toolchain-12.3.rel1-aarch64-arm-none-eabi/bin/
endif
# Strip off the trailing '-', resulting in arm-none-eabi
CROSS_COMPILE_STRIP := $(CROSS_COMPILE:%-=%)

# Build tool names.
CC = $(CROSS_COMPILE_PATH)$(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE_PATH)$(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE_PATH)$(CROSS_COMPILE)ld
AS = $(CROSS_COMPILE_PATH)$(CROSS_COMPILE)as
AR = $(CROSS_COMPILE_PATH)$(CROSS_COMPILE)ar
OBJCOPY = $(CROSS_COMPILE_PATH)$(CROSS_COMPILE)objcopy
SIZE = $(CROSS_COMPILE_PATH)$(CROSS_COMPILE)size
OBJDUMP = $(CROSS_COMPILE_PATH)$(CROSS_COMPILE)objdump

# Ask the compiler for its version
CC_VERSION := $(shell $(CC) -dumpversion)

# Get the compiler directory. We have to go through this sillyness in order to support
# paths with spaces in their names, such as under Cygwin where the CodeSourcery compiler
# is normally installed under C:\Program Files\.
CC_PREFIX := $(shell dirname "`which $(CC)`")/..

# Standard library include paths.
LIBGCC_LDPATH = $(CC_PREFIX)/lib/gcc/$(CROSS_COMPILE_STRIP)/$(CC_VERSION)

LIBC_LDPATH = $(CC_PREFIX)/$(CROSS_COMPILE_STRIP)/lib

# System header file include paths.
CC_INCLUDE = $(CC_PREFIX)/lib/gcc/$(CROSS_COMPILE_STRIP)/$(CC_VERSION)/include
CC_INCLUDE_FIXED = $(CC_PREFIX)/lib/gcc/$(CROSS_COMPILE_STRIP)/$(CC_VERSION)/include-fixed
LIBC_INCLUDE = $(CC_PREFIX)/$(CROSS_COMPILE_STRIP)/include
LIBC_INCLUDE += $(CC_PREFIX)/$(CROSS_COMPILE_STRIP)/include/c++/$(CC_VERSION)

APPSRC_INCLUDE :=$(WORK_DIR)/apps/include
THREADX_COMMON_INCLUDE :=$(WORK_DIR)/$(THREADX_DIR)/common/inc
THREADX_PORTS_INCLUDE :=$(WORK_DIR)/$(THREADX_DIR)/ports/$(THREADX_ARCH)/$(THREADX_TOOLCHAIN)/inc

LDFLAGS = -lc -lgcc -u _printf_float -lm

LDINC += -L '$(LIBGCC_LDPATH)' -L '$(LIBC_LDPATH)'

SYSTEM_INC = \
	-isystem '$(CC_INCLUDE)' \
	-isystem '$(CC_INCLUDE_FIXED)' \
	-isystem '$(LIBC_INCLUDE)'

INCLUDES += $(foreach dir, $(APPSRC_INCLUDE), -I$(dir))
INCLUDES += $(foreach dir, $(THREADX_COMMON_INCLUDE), -I$(dir))
INCLUDES += $(foreach dir, $(THREADX_PORTS_INCLUDE), -I$(dir))

LD_FILE = $(WORK_DIR)/ldscript/amp_app.ld.S
rel_ld_file = $(basename $(subst $(WORK_DIR)/,,$(abspath $(LD_FILE))))
the_ld_file = $(addprefix $(OBJ_DIR)/,$(rel_ld_file))
the_ld_file_dir = $(dir $(the_ld_file))

SRCS_C := $(shell find $(APP_SRC) -name '*.c')
SRCS_S := $(shell find $(APP_SRC) -name '*.S')
SRCS_s := $(shell find $(APP_SRC) -name '*.s')

THREADX_SRC_c :=$(shell find $(THREADX_DIR)/common/src -name "*.c")
THREADX_SRC_S :=$(shell find $(THREADX_DIR)/ports/$(THREADX_ARCH)/$(THREADX_TOOLCHAIN)/src -name "*.S")

OBJS := $(SRCS_C:%.c=$(OBJ_DIR)/%.o) $(SRCS_S:%.S=$(OBJ_DIR)/%.o) $(SRCS_s:%.s=$(OBJ_DIR)/%.o)
THREADX_OBJS :=$(THREADX_SRC_c:%.c=$(OBJ_DIR)/THREADX_DIR/%.o) $(THREADX_SRC_S:%.S=$(OBJ_DIR)/THREADX_DIR/%.o) 

DEP_F:= $(OBJS:%.o=%d)
THREADX_F :=$(THREADX_OBJS:%.o=%d)

.PHONY: src clean

src: $(THREADX_LIB) $(BIN_NAME) $(DIS_NAME)

$(THREADX_LIB):$(THREADX_OBJS)
	$(AR) -r $@ $^ 
	@echo $(THREADX_LIB) has been created

$(ELF_NAME):$(OBJS) $(the_ld_file) 
	$(LD) -Bstatic -nostdlib \
	-T $(the_ld_file) \
	$(LDINC) \
	$(OBJS) \
	$(LDFLAGS) \
	-o $@ \
	-Map $(MAP_NAME) --cref
	@echo $(ELF_NAME) has been created	
$(DIS_NAME):$(ELF_NAME)
	@$(OBJDUMP) -h -d $< > $@
	@echo $(DIS_NAME) has been created
$(BIN_NAME):$(ELF_NAME)
	@$(OBJCOPY) --gap-fill 0x00 -I elf32-little -O binary $< $@
	@echo $(BIN_NAME) has been created	

-include $(DEP_F)
-include $(THREADX_F)

$(OBJ_DIR)/THREADX_DIR/%.o:%.S
	@echo "COMPILING THREADX SOURCE $< TO OBJECT $@"
	@mkdir -p '$(@D)'
	$(CC) $(INCLUDES) -mcpu=cortex-a9 -o $@ -c -g $^

$(OBJ_DIR)/THREADX_DIR/%.o:%.c
	@echo "COMPILING THREADX SOURCE $< TO OBJECT $@"	
	@mkdir -p '$(@D)'
	$(CC) $(INCLUDES) -mcpu=cortex-a9 -o $@ -c -g $^

$(OBJ_DIR)/%.o:%.c
	@echo "COMPILING SOURCE $< TO OBJECT $@"
	@mkdir -p '$(@D)'
	@$(CC) $(CFLAGS) $(SYSTEM_INC) $(INCLUDES) -o $@ -MMD -c $^

$(OBJ_DIR)/%.o:%.s
	@echo "COMPILING SOURCE $< TO OBJECT $@"	
	@mkdir -p '$(@D)'
	@$(CC) $(CFLAGS) $(INCLUDES) -o $@ -MMD -c $^

$(OBJ_DIR)/%.o:%.S
	@echo "COMPILING SOURCE $< TO OBJECT $@"	
	@mkdir -p '$(@D)'
	@$(CC) $(CFLAGS) $(INCLUDES) -o $@ -MMD -c $^

$(the_ld_file): $(LD_FILE)
	@echo "COMPILING SOURCE $< TO OBJECT $@"
	@mkdir -p '$(@D)'
	@$(CC) -E -P $(INCLUDES) -o $@ $<

clean:
	rm -rf $(OUTPUT)
