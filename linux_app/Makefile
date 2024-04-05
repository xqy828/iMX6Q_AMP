WORK_DIR := $(shell pwd)
HOST_ARCH :=$(shell arch)
OUTPUT := output

#CC_FLAGS += -Werror
CC_FLAGS += -Wall -Wextra
CC_FLAGS += -O0 -g -fdata-sections -ffunction-sections
CC_FLAGS += -mcpu=cortex-a9 -mfpu=neon -mfloat-abi=hard
CC_FLAGS += -ftree-vectorize -mvectorize-with-neon-quad -fno-common
CC_FLAGS += -fdiagnostics-color
CC_FLAGS += -mthumb-interwork
#CC_FLAGS += -fstack-usage -Wstack-usage=1024 

CFLAGS = $(CC_FLAGS)

LDFLAGS = -lpthread -lrt -lasan -lm
LDFLAGS += -Wl,-Map,$(MAP_NAME)

PROG_NAME = $(OUTPUT)/SW_APP
ELF_NAME = $(PROG_NAME).out
MAP_NAME = $(PROG_NAME).map
DIS_NAME = $(PROG_NAME).dis
BIN_NAME = $(PROG_NAME).bin

OBJ_DIR = $(OUTPUT)/obj
APPSRC_INCLUDE = $(WORK_DIR)/include

INCLUDES := $(foreach dir, $(APPSRC_INCLUDE), -I$(dir))

ifeq ($(HOST_ARCH),x86_64) 
	TOOLS = /opt/arm-gnu-toolchain-13.2.Rel1-x86_64-arm-none-linux-gnueabihf/bin/arm-none-linux-gnueabihf-
else ifeq ($(HOST_ARCH),aarch64) 
	TOOLS = /opt/arm-gnu-toolchain-12.3.rel1-aarch64-arm-none-linux-gnueabihf/bin/arm-none-linux-gnueabihf-
endif

AS = $(TOOLS)as
CC = $(TOOLS)gcc
LD = $(TOOLS)gcc
OBJCOPY = $(TOOLS)objcopy
OBJDUMP = $(TOOLS)objdump
SIZE = $(TOOLS)size  

SRCS_C := $(shell find $(WORK_DIR) -name '*.c')
SRCS_S := $(shell find $(WORK_DIR) -name '*.S')
SRCS_s := $(shell find $(WORK_DIR) -name '*.s')


OBJS := $(SRCS_C:%.c=$(OBJ_DIR)/%.o) $(SRCS_S:%.S=$(OBJ_DIR)/%.o) $(SRCS_s:%.s=$(OBJ_DIR)/%.o)


DEP_F:= $(OBJS:%.o=%d)

.PHONY: src clean

src: $(BIN_NAME) $(DIS_NAME)

$(ELF_NAME):$(OBJS)
	@$(LD) $(OBJS) -o $@ $(LDFLAGS)
	@echo $(ELF_NAME) has been created	
$(DIS_NAME):$(ELF_NAME)
	@$(OBJDUMP) -h -d $< > $@
	@echo $(DIS_NAME) has been created
$(BIN_NAME):$(ELF_NAME)
	@$(OBJCOPY) -O binary $< $@
	@echo $(BIN_NAME) has been created	
-include $(DEP_F)
$(OBJ_DIR)/%.o:%.c
	@echo "COMPILING SOURCE $< TO OBJECT $@"	
	@mkdir -p '$(@D)'
	@$(CC) $(CFLAGS) $(INCLUDES) -o $@  -c $^

$(OBJ_DIR)/%.o:%.s
	@echo "COMPILING SOURCE $< TO OBJECT $@"	
	@mkdir -p '$(@D)'
	@$(CC) $(CFLAGS) $(INCLUDES) -o $@  -c $^


$(OBJ_DIR)/%.o:%.S
	@echo "COMPILING SOURCE $< TO OBJECT $@"	
	@mkdir -p '$(@D)'
	@$(CC) $(CFLAGS) $(INCLUDES) -o $@  -c $^

clean:
	rm -rf $(OUTPUT)
