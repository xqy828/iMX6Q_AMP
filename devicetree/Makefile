WORK_DIR := $(shell pwd)
CPP_DTSI_DIR :=$(WORK_DIR)/CPP_DTSI
DTSI_DIR :=$(WORK_DIR)/imx6q
export ARCH=arm
export CROSS_COMPILE=arm-none-linux-gnueabihf-
#export PATH=$PATH:/opt/arm-gnu-toolchain-13.2.Rel1-x86_64-arm-none-linux-gnueabihf/bin

KERNEL_SRC = $(shell dirname $(WORK_DIR))/linux-6.5
IFLAGS += -I$(KERNEL_SRC)/include 
IFLAGS += -I$(KERNEL_SRC)/arch/arm/boot/dts/nxp/imx

DTS_PREPROCESSED := $(WORK_DIR)/DTS_PREPROCESSED
PREPROCESSED_DTS :=imx6q-custom.preprocessed.dts

dtb-y +=imx6q-custom.dtb
all:$(dtb-y)

#%.dtb: %.dts
#	$(MAKE) -C $(KERNEL_SRC) M=$(shell pwd) $@ V=1	

%.dtb: %.dts
	@mkdir -p $(DTS_PREPROCESSED)
	@echo "  CPP     $(PREPROCESSED_DTS)"
	@cpp -nostdinc $(IFLAGS) -undef -x assembler-with-cpp imx6q-custom.dts > $(DTS_PREPROCESSED)/$(PREPROCESSED_DTS)
	@echo "  DTC     $@"	
	./dtc -I dts -O dtb -o $@  $(DTS_PREPROCESSED)/$(PREPROCESSED_DTS) 


#%.dtb:%.dts
#	mkdir -p $(CPP_DTSI_DIR)
#	cpp -nostdinc $(IFLAGS) -undef -x assembler-with-cpp $(DTSI_DIR)/imx6qdl.dtsi > $(CPP_DTSI_DIR)/imx6qdl.dtsi
#	cpp -nostdinc $(IFLAGS) -undef -x assembler-with-cpp $(DTSI_DIR)/imx6qdl-sabresd.dtsi > $(CPP_DTSI_DIR)/imx6qdl-sabresd.dtsi
#	cpp -nostdinc $(IFLAGS) -undef -x assembler-with-cpp $(DTSI_DIR)/imx6q.dtsi > $(CPP_DTSI_DIR)/imx6q.dtsi
#	./dtc -I dts -O dtb -o $@  $(DTC_INCLUDE) imx6q-custom.dts
 
clean:
	rm -f *.dtb
	rm -rf $(DTS_PREPROCESSED)

.PHONY : all clean
