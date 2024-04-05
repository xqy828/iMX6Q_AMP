#!/bin/bash
start_time=$(date +%s)
export ARCH=arm
export CROSS_COMPILE=arm-none-linux-gnueabihf-
if [ $(arch) = "x86_64" ]; then
    export PATH=$PATH:/opt/arm-gnu-toolchain-13.2.Rel1-x86_64-arm-none-linux-gnueabihf/bin
elif [ $(arch) = "aarch64" ]; then
    export PATH=$PATH:/opt/arm-gnu-toolchain-12.3.rel1-aarch64-arm-none-linux-gnueabihf/bin
fi
echo make distclen
make distclean
echo make imx_v6_v7_defconfig
make imx_v6_v7_defconfig
#make menuconfig
echo make
make -j8   
end_time=$(date +%s)
cost_time=$[ $end_time-$start_time ]
echo "build kernel time is $(($cost_time/60))min $(($cost_time%60))s"
