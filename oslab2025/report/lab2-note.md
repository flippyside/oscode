自制OS的CPU：Intel 80386
模拟80386平台的虚拟机： QEMU
交叉编译的编译器： GCC 调试工具：GDB
QEMU，GCC，GDB的运行平台： Linux
编程语言： C，x86 Assembly，Makefile

lab2:

- debug范式：专门编写一个c文件，用putchar输出
- 读懂框架
- 警惕elf加载
- 理解fork、sleep函数
- 内核栈的处理，手册有现成代码，请务必理解


要求：
- 内核 kernel
- 用户态、内核态
- 中断机制、系统调用
  - IDT、TSS、isr

实验流程
1.由实模式开启保护模式并跳转到bootloader (start.s)
2.由bootloader加载kernel (boot.c)
3.完善kernel相关的初始化设置
4.由kernel加载用户程序
5.实现用户需要的库函数
6.用户程序调用自定义实现的库函数完成格式化输入输出，通过测试代码

OS 的启动顺序:

初始化串口输出

初始化中断向量表（initIdt()）

初始化8259a中断控制器（initIntr()）

初始化 GDT、配置 TSS 段（initSeg()）

初始化VGA设备（initVga()）

配置好键盘映射表（initKeyTable()）

从磁盘加载用户程序到内存相应地址（loadUMain()）

进入用户空间（enterUserSpace()）

调用库函数 sleep(), printf(), getChar(), getStr()


disk.c 添加写磁盘扇区

# bootloader加载kernel



# 中断机制

为实现用户态服务和保护特权级代码，需要完善中断机制，具体包括优化和实现 IDT（中断描述符表）、TSS（任务状态段）以及中断处理程序等关键结构。

具体实现：
- initIdt，让中断号与中断处理程序绑定。在idt中加上该中断对应的门描述符
- 实现该中断的处理函数 `irqKeyboard`，位于doirq.s，以及其调用的`handle`函数，位于irqhandle.c


## idt.c

初始化interrupt gate
```c
static void setIntr(struct GateDescriptor *ptr, uint32_t selector, uint32_t offset, uint32_t dpl) {
	// TODO: 初始化interrupt gate
	ptr->offset_15_0 = offset;
	ptr->offset_31_16 = offset >> 16;
	ptr->segment = selector;
	ptr->type = 0xe; // 01110
	ptr->privilege_level = dpl;
	ptr->present = 1;
}
```

initIdt

将函数写到对应向量号的idt表中，并填上段选择子`SEG_KCODE`.Page Fault, Double Fault, GPF使用 setTrap(),其余用setIntr。irqSyscall级别为用户级







