# imx6q amp

### Project Title
imx6q amp simplified version    

 **Linux on cpu0-cpu2  
ThreadX applications on cpu3**    

### Development platform:  
 Raspberry Pi 4B   
### Software development tools:  
 arm-none-eabi-gcc gcc version 12.3.1 20230626 (Arm GNU Toolchain 12.3.Rel1 (Build arm-12.35))    
 arm-none-linux-gnueabihf-gcc gcc version 12.3.1 20230626 (Arm GNU Toolchain 12.3.Rel1 (Build arm-12.35))   
### Reference Documents:  
 <<i.MX 6Dual/6Quad Applications Processor Reference Manual>>   
### Reference code:  
```c
 iMX6_Platform_SDK     
 https://github.com/antirez/linenoise 
 https://github.com/eclipse-threadx/threadx 
```
### Development board:  
![输入图片说明](doc/e9v2.jpg)
![输入图片说明](doc/e9v2_1.jpg)

### Software architecture

![输入图片说明](doc/software_architecture.png)

## amp ctrl  
### Test
```c
[root@imx6q /usr/local/app]# ls
SW_APP.out    cpu3_app.elf
[root@imx6q /usr/local/app]# ./SW_APP.out &
[root@imx6q /usr/local/app]# 
App start ...
Build Time:Jan 19 2025-12:39:46.
load /usr/local/app/cpu3_app.elf to Stream buf , size 4673932. 
Loading ELF headering
Loading ELF program headering
Loading ELF section header.
Loading ELF shstrtab.
/usr/local/app/cpu3_app.elf convert to bin success, bin size 4440064. 
save /usr/local/app/cpu3_app.bin bin success, bin size 4440064. 
load cpu3 elf app success , size 4440064. 
cpu3 entry addr:0x78000440,entry arg:0x7801aa20 
cpu3 power on 
cpu3 soft uart addr:0x78035000
cpu3 soft uart vir addr: 0xb644c000 
softuart thread start ...
[-CPU3-]:main-(00045)]Build Time:Jan 25 2025-09:59:22.
[-CPU3-]:main-(00046)]float test pi = 3.141593
[-CPU3-]:main-(00047)]process debug addr:0x78034458
[-CPU3-]:main-(00050)]cpsr reg = 0x400001d3
[-CPU3-]:main-(00052)]vbar reg = 0x78000440
[-CPU3-]:disp_scu_all_regs-(00030)]SCU_Control_Register = 0x00000021
[-CPU3-]:disp_scu_all_regs-(00032)]SCU_Configuration_Register = 0x00005573
[-CPU3-]:disp_scu_all_regs-(00034)]SCU_CPU_Power_Status_Register = 0x00000000
[-CPU3-]:disp_scu_all_regs-(00036)]SCU_Invalidate_All_Registers_in_Secure_State = 0x00000000
[-CPU3-]:disp_scu_all_regs-(00038)]Filtering_Start_Address_Register = 0x00000000
[-CPU3-]:disp_scu_all_regs-(00040)]Filtering_End_Address_Register = 0x00000000
[-CPU3-]:disp_scu_all_regs-(00042)]SCU_Access_Control_Register = 0x0000000f
[-CPU3-]:disp_scu_all_regs-(00044)]SCU_Non-secure_Access_Control_Register = 0x00000000
[-CPU3-]:mmu_table_init-(00273)]38 Items translation table init done 
[-CPU3-]:main-(00077)]Enable MMU 
[-CPU3-]:main-(00080)]Enable SIMD VFP 
[-CPU3-]:main-(00083)]Neon Test ...
I:  7298-Q: -8657|I: 28513-Q:  7192|I:-14294-Q:  4935|I:  3797-Q: 25403|I:-13302-Q: 18999|I: 17846-Q:-31606|I:  3377-Q: -1452|I:   690-Q: 10603|I: -2352-Q:-20951|I:-24549-Q:   521|I:  8485-Q:  2026|I:-10757-Q: 30099|I:  5764-Q:  1393|I: -1091-Q:  5758|I: 
[-CPU3-]:TestNeon-(00059)]C Intrinsics done 
I:  7298-Q: -8657|I: 28513-Q:  7192|I:-14294-Q:  4935|I:  3797-Q: 25403|I:-13302-Q: 18999|I: 17846-Q:-31606|I:  3377-Q: -1452|I:   690-Q: 10603|I: -2352-Q:-20951|I:-24549-Q:   521|I:  8485-Q:  2026|I:-10757-Q: 30099|I:  5764-Q:  1393|I: -1091-Q:  5758|I: 
[-CPU3-]:TestNeon-(00095)]NEON Intrinsics done 
[-CPU3-]:TestNeon-(00096)]int convert to  short test 
[-CPU3-]:TestNeon-(00097)]RAW Data: 
25791 531569 -2334455 -11515 
[-CPU3-]:TestNeon-(00103)]C: 
25791 7281 24841 -11515 
[-CPU3-]:TestNeon-(00110)]NEON:
25791 7281 24841 -11515 
[-CPU3-]:TestNeon-(00118)]NEON saturated:
25791 32767 -32768 -11515 
[-CPU3-]:main-(00085)]Normal Distribution Random number Test ...
    2.8997211   -0.9088573    0.2041950   -0.2572155   -0.8516827    0.7996998   -0.9866619    0.0431385   -1.9194927    0.2543507
   -0.3689251    1.2145863   -1.0537090    1.7050953   -1.6925945   -0.4928722    1.9956684   -0.5980663    1.2923298    0.1707630
   -0.5213604   -0.4051342    0.8358479   -0.5445080    1.6452045    0.5338917   -0.8120403   -0.3886852   -0.2546368    0.4690113
   -0.4013348   -0.1117687   -0.9708843    0.6502247    1.3179646    0.5362415    0.7464619    1.3275318   -0.4041424    1.8053455
   -0.8525982   -0.2490673    1.6823444    0.9455433    0.4819355    1.1704273   -0.1725750    0.2068348   -1.9999371    0.8360157
[-CPU3-]:application_main-(00061)]ThreadX Version:06.04.00.
[-CPU3-]:thread_0_entry-(00191)]**** ThreadX Demonstration on NXP i.MX 6Quad **** 

           Thread 0 Counter:            0x00000001
           Thread 1 Counter:            0x00000000
           Thread 2 Counter:            0x00000000
           Thread 3 Counter:            0x00000000
           Thread 4 Counter:            0x00000000
           Thread 5 Counter:            0x00000000
           Thread 6 Counter:            0x00000000
           Thread 7 Counter:            0x00000000

[-CPU3-]:thread_0_entry-(00191)]**** ThreadX Demonstration on NXP i.MX 6Quad **** 

           Thread 0 Counter:            0x00000002
           Thread 1 Counter:            0x0022aacb
           Thread 2 Counter:            0x0022aaa0
           Thread 3 Counter:            0x000000fb
           Thread 4 Counter:            0x000000fa
           Thread 5 Counter:            0x00000001
           Thread 6 Counter:            0x000000fb
           Thread 7 Counter:            0x000000fa

[-CPU3-]:thread_0_entry-(00191)]**** ThreadX Demonstration on NXP i.MX 6Quad **** 

           Thread 0 Counter:            0x00000003
           Thread 1 Counter:            0x0045592d
           Thread 2 Counter:            0x004558ee
           Thread 3 Counter:            0x000001f5
           Thread 4 Counter:            0x000001f4
           Thread 5 Counter:            0x00000002
           Thread 6 Counter:            0x000001f5
           Thread 7 Counter:            0x000001f4

```
## xshell debug tools 
### Software architecture   
![输入图片说明](doc/xshell.png)    

### Test    
```c
[root@imx6q /mnt/emmc]# [  287.472044] imx6q-amp imx6q_amp: virq sgi 289 kick cpu:0
[-CPU3-]:main-(00092)][  287.477420] imx6q-amp imx6q_amp: work executed on CPU 0
run times:0x00000016.
[root@imx6q /mnt/emmc]# xshell.out 
Current tty: /dev/ttymxc0
APP tty is redirect to /dev/ttymxc0
xshell>>
xshell>>Cpu3ElfHeader_Show
ELF Header:
  Magic:   7f 45 4c 46 01 01 01 00 00 00 00 00 00 00 00 00 
  Class:                             ELF32
  Data:                              2's complement, little endian
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI Version:                       0
  Type:                              EXEC (Executable file)
  Machine:                           ARM
  Version:                           0x1
  Entry point address:               0x78000440
  Start of program headers:          52 (bytes into file)
  Start of section headers:          4673036 (bytes into file)
  Size of this header:               52 (bytes)
  Size of program headers:           32 (bytes)
  Number of program headers:         4  Size of section headers:           40 (bytes)
  Number of section headers:         31
  Section header string table index: 30
Section Headers:
  [Nr]  Name             Type             Addr       Off        Size       ES   Flg Lk Inf Al
  [ 0]                   NULL             0x00000000 0x00000000 0x00000000 0x00
  [ 1] .ivt              PROGBITS         0x78000000 0x00001000 0x00000420 0x00
  [ 2] .boot_data        PROGBITS         0x78000420 0x00001420 0x0000000c 0x00
  [ 3] .dcd_hdr          PROGBITS         0x78000430 0x00001430 0x00000004 0x00
  [ 4] .dcd_wrt_cmd      PROGBITS         0x78000434 0x00001434 0x00000004 0x00
  [ 5] .text             PROGBITS         0x78000438 0x00001438 0x0001a6e0 0x00
  [ 6] .cpu3main         PROGBITS         0x7801ab18 0x0001bb18 0x00000328 0x00
  [ 7] .rodata           PROGBITS         0x7801ae40 0x0001be40 0x00001724 0x00
  [ 8] .eh_frame         PROGBITS         0x7801c564 0x0001d564 0x00000028 0x00
  [ 9] .ARM.exidx        ARM_EXIDX        0x7801c58c 0x0001d58c 0x00000008 0x00
  [10] .data             PROGBITS         0x7801c598 0x0001d598 0x00018aec 0x00
  [11] .cpu3softuart     PROGBITS         0x78036000 0x00037000 0x00000008 0x00
  [12] .mmu_l1_table     PROGBITS         0x78038000 0x00038000 0x00004000 0x00
  [13] .mmu_l2_table     PROGBITS         0x7803c000 0x0003c000 0x00400000 0x00
  [14] .hab.data         NOBITS           0x78a00000 0x0043c000 0x00002000 0x00
  [15] .bss              NOBITS           0x78a02000 0x0043c000 0x000095d8 0x00
  [16] .heap             NOBITS           0x78a0b5d8 0x0043c000 0x01000000 0x00
  [17] .stacks           NOBITS           0x79a0b5d8 0x0043c000 0x00a02808 0x00
  [18] .ARM.attributes   ARM_ATTRIBUTES   0x00000000 0x0043c000 0x00000035 0x00
  [19] .comment          PROGBITS         0x00000000 0x0043c035 0x00000044 0x01
  [20] .debug_info       PROGBITS         0x00000000 0x0043c079 0x00019073 0x00
  [21] .debug_abbrev     PROGBITS         0x00000000 0x004550ec 0x000056bc 0x00
  [22] .debug_aranges    PROGBITS         0x00000000 0x0045a7a8 0x00000ac0 0x00
  [23] .debug_line       PROGBITS         0x00000000 0x0045b268 0x000094a3 0x00
  [24] .debug_str        PROGBITS         0x00000000 0x0046470b 0x00003f81 0x01
  [25] .debug_frame      PROGBITS         0x00000000 0x0046868c 0x00003a2c 0x00
  [26] .debug_rnglists   PROGBITS         0x00000000 0x0046c0b8 0x00000034 0x00
  [27] .debug_line_str   PROGBITS         0x00000000 0x0046c0ec 0x000002be 0x01
  [28] .symtab           SYMTAB           0x00000000 0x0046c3ac 0x00005910 0x10
  [29] .strtab           STRTAB           0x00000000 0x00471cbc 0x00003013 0x00
  [30] .shstrtab         STRTAB           0x00000000 0x00474ccf 0x0000013b 0x00

Program Headers:
  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
  EXIDX          0x01d58c 0x7801c58c 0x7801c58c 0x00008 0x00008 R   0x4
  LOAD           0x001000 0x78000000 0x78000000 0x36008 0x36008 RWE 0x1000
  LOAD           0x038000 0x78038000 0x78038000 0x404000 0x404000 RW  0x4000
  LOAD           0x000000 0x78a00000 0x78a00000 0x00000 0x1a0dde0 RW  0x1000
addr:0x00016081.
xshell>>
xshell>>exit
exit xshell !
[root@imx6q /mnt/emmc]# 
```
