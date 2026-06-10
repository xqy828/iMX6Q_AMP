export ARCH=arm
export CROSS_COMPILE=/opt/arm-gnu-toolchain-13.2.Rel1-x86_64-arm-none-linux-gnueabihf/bin/arm-none-linux-gnueabihf-

CC = $(CROSS_COMPILE)gcc

KBUILD_EXTRA_SYNBOLS += /home/linux/workspace/imx6q_amp/linux-6.5/Module.symvers
export KBUILD_EXTRA_SYNBOLS
KERNEL_DIR = /home/linux/workspace/imx6q_amp/linux-6.5
CUR_DIR = $(shell pwd)
OUTPUT_DIR ?= $(CUR_DIR)/out
OBJ_DIR ?= $(CUR_DIR)/obj
SRC_DIR ?= $(CUR_DIR)

export CC KERNEL_DIR OBJ_DIR OUTPUT_DIR

all: module

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)
	mkdir -p $(OBJ_DIR)

module: $(OUTPUT_DIR)
	make -C $(KERNEL_DIR) M=$(SRC_DIR) modules
	@mv *.ko $(OUTPUT_DIR)
	@mv *.o $(OBJ_DIR) 2>/dev/null || true
	@mv *.symvers $(OBJ_DIR) 2>/dev/null || true
	@mv *.order $(OBJ_DIR) 2>/dev/null || true
	@mv *.mod.c $(OBJ_DIR) 2>/dev/null || true

clean:
	@rm -f $(OBJ_DIR)/*.o $(OBJ_DIR)/*.mod.* $(OBJ_DIR)/*.symvers $(OBJ_DIR)/*.order
	@rm -f $(OUTPUT_DIR)/*.ko
	@rm -f *.mod *.o *.mod.c *.symvers *.order

.PHONY: clean module
