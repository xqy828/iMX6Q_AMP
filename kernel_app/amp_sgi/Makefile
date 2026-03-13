export ARCH=arm
export CROSS_COMPILE=/opt/arm-gnu-toolchain-13.2.Rel1-x86_64-arm-none-linux-gnueabihf/bin/arm-none-linux-gnueabihf-

#TOOL=/opt/Petalinux/2018.3/tools/linux-i386/gcc-arm-linux-gnueabi/bin/arm-linux-gnueabihf
CC = $(CROSS_COMPILE)gcc
GLOBAL_CFLAGS = -Werror -std=gnu99 -g -O0

KBUILD_EXTRA_SYNBOLS += /home/linux/workspace/imx6q_amp/linux-6.5/Module.symvers
export KBUILD_EXTRA_SYNBOLS 
KERNEL_DIR = /home/linux/workspace/imx6q_amp/linux-6.5
CUR_DIR = $(shell pwd)
OUTPUT_DIR ?= $(CUR_DIR)/out
OBJ_DIR ?= $(CUR_DIR)/obj
SRC_DIR ?= $(CUR_DIR)/src

PWD = $(shell pwd)

export CC PWD KERNEL_DIR OBJ_DIR OUTPUT_DIR

all:src 

include src/src.mk

$(OUTPUT_DIR):
	$(shell mkdir -p $(OUTPUT_DIR))\
	$(shell mkdir -p $(OBJ_DIR))

#$(OBJ_DIR):
#	$(shell mkdir -p $(OBJ_DIR))

clean:
	@(if [ -d $(OBJ_DIR) ]; then\
		rm -f $(OBJ_DIR)/*.o;\
		rm -f $(OBJ_DIR)/*.mod.*;\
		rm -f $(OBJ_DIR)/*.symvers;\
		rm -f $(OBJ_DIR)/*.order;\
		find $(OBJ_DIR) -maxdepth 1 -name ".*" ! -name ".gitkeep" ! -name "." ! -name ".." -exec rm -rf {} + ;\
	fi)
	@(if [ -d $(OUTPUT_DIR) ]; then\
		rm -f $(OUTPUT_DIR)/*.ko;\
	fi)
	@(rm -f $(SRC_DIR)/*.mod)

.PHONY: clean

