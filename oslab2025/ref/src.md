期望的提交文件结构为：

lab2-STUID                     #自行修改后打包(.zip)提交

├── lab

│   ├── Makefile

│   ├── app                    #用户代码

│   │   ├── Makefile

│   │   └── main.c             #主函数

│   ├── bootloader             #引导程序

│   │   ├── Makefile

│   │   ├── boot.c

│   │   ├── boot.h

│   │   └── start.S

│   ├── kernel

│   │   ├── Makefile

│   │   ├── include             #头文件

│   │   │   ├── common

│   │   │   │   ├── assert.h

│   │   │   │   ├── const.h

│   │   │   │   └── types.h

│   │   │   ├── common.h

│   │   │   ├── device

│   │   │   │   ├── disk.h

│   │   │   │   ├── keyboard.h

│   │   │   │   ├── serial.h

│   │   │   │   ├── timer.h

│   │   │   │   └── vga.h

│   │   │   ├── device.h

│   │   │   ├── x86

│   │   │   │   ├── cpu.h

│   │   │   │   ├── io.h

│   │   │   │   ├── irq.h

│   │   │   │   └── memory.h

│   │   │   └── x86.h

│   │   ├── kernel             #内核代码

│   │   │   ├── disk.c         #磁盘读写API

│   │   │   ├── doIrq.S        #中断处理

│   │   │   ├── i8259.c        #重设主从8259A

│   │   │   ├── idt.c          #初始化中断描述

│   │   │   ├── irqHandle.c    #中断处理函数

│   │   │   ├── keyboard.c     #初始化键码表

│   │   │   ├── kvm.c          #初始化 GDT 和加载用户程序

│   │   │   ├── serial.c       #初始化串口输出

│   │   │   ├── timer.c        #设置8253/4定时器芯片

│   │   │   └── vga.c

│   │   ├── lib

│   │   │   └── abort.c

│   │   ├── main.c             #主函数

│   │   └── Makefile

│   ├── lib                    #库函数

│   │   ├── lib.h

│   │   ├── syscall.c          #系统调用入口

│   │   └── types.h

│   ├── Makefile

│   └── utils

│       ├── genBoot.pl             #生成引导程序

│       └── genKernel.pl           #生成内核程序

└── report

        └── 231220000.pdf



