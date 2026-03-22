HOST_ARCH :=$(shell arch)
ifeq ($(HOST_ARCH),x86_64)
        CC = /opt/arm-gnu-toolchain-13.2.Rel1-x86_64-arm-none-linux-gnueabihf/bin/arm-none-linux-gnueabihf-gcc
else ifeq ($(HOST_ARCH),aarch64)
        CC = /opt/arm-gnu-toolchain-12.3.rel1-aarch64-arm-none-linux-gnueabihf/bin/arm-none-linux-gnueabihf-gcc
endif

CFLAGS=-Wall -g
LDFLAGS=-lpthread -lrt
DLIBS=-lm

SUBDIRS=$(shell ls -l | grep ^d |awk  '{if($$9!="out" && $$9=="src") print $$9}')
#OUT=$(shell ls -l | grep ^d |awk  '{if($$9=="out") print $$9}')
PRO_DIR=$(shell pwd)
SRC_DIR:=$(shell find $(PRO_DIR) -maxdepth 3 -type d)
OUTPUT_DIR=$(PRO_DIR)/out
OBJ_DIR=$(OUTPUT_DIR)/obj
BIN_DIR=$(OUTPUT_DIR)/bin
INCLUDE_DIR=$(PRO_DIR)/inc
src=$(foreach dir, $(SRC_DIR), $(wildcard $(dir)/*.c))
obj=$(patsubst %.c, %.o, $(notdir $(src)))
target=xshell.out

CFLAGS += $(foreach dir, $(INCLUDE_DIR), -I$(dir))

export CC CFLAGS LDFLAGS DLIBS OUTPUT_DIR OBJ_DIR BIN_DIR PRO_DIR target

all:$(SUBDIRS) DEBUG

$(SUBDIRS):ECHO
	make -C $@
DEBUG:ECHO
	make -C $(OUTPUT_DIR)
ECHO:
	@echo $(SUBDIRS)
#==============compile=============
#$(obj):%.o:%.c#变量替换.C生成对应的.O文件
#	$(CC) $(CFLAGS) -o $(OBJ_DIR)/$@ -c $^
.PHONY:clean
clean:
	rm -rf $(OBJ_DIR)/*.o $(BIN_DIR)/*.out
