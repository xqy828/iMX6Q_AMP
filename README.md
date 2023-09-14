# imx6q_amp

#### 介绍
Linux on cpu0-cpu2  
Bare-metal applications on cpu3  

#### 软件架构
软件架构说明


#### 安装教程

1.  xxxx
2.  xxxx
3.  xxxx

#### 测试
```
[root@imx6q /usr/local/app]# ls
SW_APP.out    cpu3_app.bin  cpu3_app.elf
[root@imx6q /usr/local/app]# ./SW_APP.out &
[root@imx6q /usr/local/app]# App start ...

load /usr/local/app/cpu3_app.elf to Stream buf , size 425024.
Loading ELF headering
Loading ELF program headering
Loading ELF section header.
Loading ELF shstrtab.
load cpu3 app to Stream buf , size 135648.
load /usr/local/app/cpu3_app.bin success .
cpu3 entry addr:0x78000000,entry arg:0x7801f894
cpu3 power on
cpu3 soft uart addr:0x780211d8
CPU3 SoftUartInit Vir addr: 0xb64371d8
softuart thread start ...
CPU3: [-CPU3-]:Build Time:Sep 14 2023-07:29:20.
CPU3: [-CPU3-]:run times:0x00000000.
CPU3: [-CPU3-]:run times:0x00000001.
CPU3: [-CPU3-]:run times:0x00000002.
CPU3: [-CPU3-]:run times:0x00000003.
CPU3: [-CPU3-]:run times:0x00000004.
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
