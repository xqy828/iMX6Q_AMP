export ARCH=arm
CROSS_COMPILE ?= /opt/arm-gnu-toolchain-13.2.Rel1-x86_64-arm-none-linux-gnueabihf/bin/arm-none-linux-gnueabihf-
CC = $(CROSS_COMPILE)gcc

CFLAGS = -Werror -std=gnu99 -g -O0

OUTPUT_DIR ?= $(shell pwd)/out
SRC = $(shell pwd)/reboot.c
TARGET = $(OUTPUT_DIR)/reboot.out

all: $(TARGET)

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

$(TARGET): $(SRC) $(OUTPUT_DIR)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	@rm -f $(TARGET)

.PHONY: all clean
