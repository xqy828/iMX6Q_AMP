# imx6q amp

### Project Title
imx6q amp simplified version    

 **Linux on cpu0-cpu2  
Bare-metal applications on cpu3**    

### Development platform:  
 Raspberry Pi 4B   
### Software development tools:  
 arm-none-eabi-gcc gcc version 12.3.1 20230626 (Arm GNU Toolchain 12.3.Rel1 (Build arm-12.35))    
 arm-none-linux-gnueabihf-gcc gcc version 12.3.1 20230626 (Arm GNU Toolchain 12.3.Rel1 (Build arm-12.35))   
### Reference Documents:  
 <<i.MX 6Dual/6Quad Applications Processor Reference Manual>>   
### Reference code:  
 iMX6_Platform_SDK  
### Development board:  
![输入图片说明](doc/e9v2.jpg)
![输入图片说明](doc/e9v2_1.jpg)

### Software architecture

![输入图片说明](doc/software_architecture.png)

### Test
```
[root@imx6q /usr/local/app]# ls
SW_APP.out    cpu3_app.elf
[root@imx6q /usr/local/app]# ./SW_APP.out &
[root@imx6q /usr/local/app]#
App start ...
Build Time:Nov 18 2023-19:31:03.
load /usr/local/app/cpu3_app.elf to Stream buf , size 215340.
Loading ELF headering
Loading ELF program headering
Loading ELF section header.
Loading ELF shstrtab.
/usr/local/app/cpu3_app.elf convert to bin success, bin size 166284.
load cpu3 elf app success , size 166284.
cpu3 entry addr:0x78000438,entry arg:0x7800f634
cpu3 power on
cpu3 soft uart addr:0x78028984
cpu3 soft uart vir addr: 0xb650c984
softuart thread start ...
[-CPU3-]:Build Time:Nov 19 2023-21:05:17.
[-CPU3-]:float test pi = 3.141593
[-CPU3-]:Enable SIMD VFP
[-CPU3-]:Neon Test ...
I:  7298-Q: -8657|I: 28513-Q:  7192|I:-14294-Q:  4935|I:  3797-Q: 25403|I:-13302-Q: 18999|I: 17846-Q:-31606|I:  3377-Q: -1452|I:   690-Q: 10603|I: -2352-Q:-20951|I:-24549-Q:   521|I:  8485-Q:  2026|I:-10757-Q: 30099|I:  5764-Q:  1393|I: -1091-Q:  5758|I:
[-CPU3-]:C Intrinsics done
I:  7298-Q: -8657|I: 28513-Q:  7192|I:-14294-Q:  4935|I:  3797-Q: 25403|I:-13302-Q: 18999|I: 17846-Q:-31606|I:  3377-Q: -1452|I:   690-Q: 10603|I: -2352-Q:-20951|I:-24549-Q:   521|I:  8485-Q:  2026|I:-10757-Q: 30099|I:  5764-Q:  1393|I: -1091-Q:  5758|I:
[-CPU3-]:NEON Intrinsics done
[-CPU3-]:int convert to  short test
[-CPU3-]:RAW Data:
25791 531569 -2334455 -11515
[-CPU3-]:C:
25791 7281 24841 -11515
[-CPU3-]:NEON:
25791 7281 24841 -11515
[-CPU3-]:NEON saturated:
25791 32767 -32768 -11515
[-CPU3-]:Normal Distribution Random number Test ...
    2.8997211   -0.9088573    0.2041950   -0.2572155   -0.8516827    0.7996998   -0.9866619    0.0431385   -1.9194927    0.2543507
   -0.3689251    1.2145863   -1.0537090    1.7050953   -1.6925945   -0.4928722    1.9956684   -0.5980663    1.2923298    0.1707630
   -0.5213604   -0.4051342    0.8358479   -0.5445080    1.6452045    0.5338917   -0.8120403   -0.3886852   -0.2546368    0.4690113
   -0.4013348   -0.1117687   -0.9708843    0.6502247    1.3179646    0.5362415    0.7464619    1.3275318   -0.4041424    1.8053455
   -0.8525982   -0.2490673    1.6823444    0.9455433    0.4819355    1.1704273   -0.1725750    0.2068348   -1.9999371    0.8360157
[-CPU3-]:run times:0x00000000.
[-CPU3-]:run times:0x00000001.

```

#### 参与贡献

1.  Fork 本仓库
2.  新建 Feat_xxx 分支
3.  提交代码
4.  新建 Pull Request


#### 特技

1.  使用 Readme\_XXX.md 来支持不同的语言，例如 Readme\_en.md, Readme\_zh.md
2.  Gitee 官方博客 [blog.gitee.com](https://blog.gitee.com)
3.  你可以 [https://gitee.com/explore](https://gitee.com/explore) 这个地址来了解 Gitee 上的优秀开源项目
4.  [GVP](https://gitee.com/gvp) 全称是 Gitee 最有价值开源项目，是综合评定出的优秀开源项目
5.  Gitee 官方提供的使用手册 [https://gitee.com/help](https://gitee.com/help)
6.  Gitee 封面人物是一档用来展示 Gitee 会员风采的栏目 [https://gitee.com/gitee-stars/](https://gitee.com/gitee-stars/)
