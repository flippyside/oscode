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

初始化串口输出 `serial.c` 
初始化中断向量表（initIdt()） `idt.c`
初始化8259a中断控制器（initIntr()）`i8259.c`
初始化 GDT、配置 TSS 段（initSeg()）`kvm.c`
初始化VGA设备（initVga()）`vga.c`
配置好键盘映射表（initKeyTable()）`keyboard.c`
从磁盘加载用户程序到内存相应地址（loadUMain()） `kvm.c` todo
进入用户空间（enterUserSpace()）`kvm.c`
调用库函数 sleep(), printf(), getChar(), getStr()



# 代码改动

- 在disk.c 添加写磁盘扇区
- kEntry，一系列初始化
- irqHandle
- bootMain，由bootloader加载kernel
- loadUMain，由kernel加载用户程序
- sysPrint
- movl $0x7c00,%eax        #  setting esp
- 实现printf() 
- KeyboardHandle，实现向命令行中输入字符
- getChar(), getStr()
- now


# bootloader加载kernel

## elf

ELF（Executable and Linkable Format）：是Linux上常见的可执行文件格式，

主要组成部分：
- ELF Header（头部）：描述整个文件的基本信息，如字节序、架构、入口点等。
- Program Header（程序头表）：用于告诉内核如何将程序加载到内存。
- Section Header（段头表）：用于链接器和调试器，标识各种段的信息。
- Segments（段）：按内存布局划分的程序代码、数据等。
- Sections（节）：更精细的划分，包含符号表、字符串表、调试信息等。

操作系统加载 ELF 的流程：

1. 启动引导（Boot Loader）
- 计算机启动时，BIOS加载引导程序。
- 引导程序加载内核镜像（通常也是一个ELF文件）到内存。

2. 加载内核
- 引导程序读取ELF头部，找到入口点（e_entry字段）。
- 根据**程序头表（Program Header Table）**的指引，将各个段（如代码段、数据段）加载到相应的内存地址。
- 内核初始化并准备切换到保护模式（Protected Mode）。


3. 执行内核
- 启动内核的入口点（e_entry指定的地址）。
- 通常该入口点会启动内核的初始化函数，设置内存管理、初始化设备驱动、挂载根文件系统等。


4. 加载用户空间程序

用户程序地址 = 0x200000;



# 进入用户空间

loadUMain 加载用户程序：
- 读取 ELF 文件并将其内容加载到内存。
- 获取 ELF 文件的入口地址。

enterUserSpace 执行用户程序：
- 设置段寄存器以切换到用户模式
- 使用 iret 指令跳转到用户程序入口，开始执行用户代码

# 中断机制

为实现用户态服务和保护特权级代码，需要完善中断机制，具体包括优化和实现 IDT（中断描述符表）、TSS（任务状态段）以及中断处理程序等关键结构。

具体实现：
- initIdt，让中断号与中断处理程序绑定。在idt中加上该中断对应的门描述符
- 实现该中断的处理函数 `irqKeyboard`，位于doirq.s，以及其调用的`handle`函数，位于irqhandle.c

调用过程：
irqSyscall -> asmDoIrq -> irqHandle -> KeyboardHandle

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

将函数写到对应向量号的idt表中，并填上段选择子`SEG_KCODE`.

Page Fault, Double Fault, GPF使用 setTrap(),其余用setIntr。irqSyscall级别为用户级

## irqhandle


```c
struct TrapFrame {
	uint32_t edi, esi, ebp, xxx, ebx, edx, ecx, eax;
	int32_t irq;
}; 
```

这些字段是x86架构下的寄存器，`TrapFrame`结构通常用于中断或异常处理，保存CPU的上下文。各字段的含义如下：

- `edi`, `esi`, `ebp`, `ebx`, `edx`, `ecx`, `eax`：这些是x86的通用寄存器，分别对应于寄存器`%edi`, `%esi`, `%ebp`, `%ebx`, `%edx`, `%ecx`, `%eax`。在中断或异常时，这些寄存器的值会被保存在这里，以便后续恢复。  
- `xxx`：这个字段不是标准的x86寄存器，可能是某个保留位、对齐用的占位字段，或者是系统实现中的特殊寄存器。具体作用需要结合代码上下文。  
- `irq`：中断号，标识当前触发的中断或异常的类型。它通常是一个负值表示异常，正值表示硬件中断。  

这种结构一般会作为内核栈的一部分，帮助中断服务例程（ISR）恢复之前的CPU状态。



# 系统调用

sleep(), printf(), getChar(), getStr(), now()

## KeyboardHandle

辅助函数print_out：
```c
void print_out(int displayRow, int displayCol, char character){
	int vga_mem = 0xB8000; // VGA 显存起始地址
	int color = 0x0c;
	int pos = (displayRow * 80 + displayCol) * 2;
	*(char*)(vga_mem + pos) = character;
	*(char*)(vga_mem + pos + 1) = color;
}
```


将用户输入的字符存入键盘缓冲区并打印到屏幕。
```c
			keyBuffer[bufferTail++] = ch;
			asm volatile("movw %0, %%es"::"m"(sel)); // 将 sel 的值加载到段寄存器 ES 中。
			print_out(displayRow, displayCol, ch);
```

## syscall

参数从左到右，依次放入：eax ecx edx ebx esi edi

int $0x80

以eax作为返回值

## 内核栈的处理

如何从用户栈切换到内核栈



## print

printf `syscall.c` -> syscall(`int $0x80`) -> irqHandle -> syscallHandle -> sysPrint `iqrhandle.c`

printf实现：
- 在3种状态中切换，0表示%，1表示format，2表示普通字符
- 对于四种format，应用已有的宏定义和转换函数
- 如果buffer已满，就系统调用打印出buffer中的内容

```c
	while(format[i]!=0){
		// TODO: support format %d %x %s %c
		char ch = format[i++];

		switch(state) {
			case 0:
				if (ch == '%') state = 1;
				else buffer[count++] = ch;
				break;
			case 1:
				if (ch == 'd') {
					count = dec2Str(va_arg(paraList, int), buffer, MAX_BUFFER_SIZE, count);
				}
				else if (ch == 'x') {
					count = hex2Str(va_arg(paraList, uint32_t), buffer, MAX_BUFFER_SIZE, count);
				}
				else if (ch == 's') {
					count = str2Str(va_arg(paraList, char *), buffer, MAX_BUFFER_SIZE, count);
				}
				else if (ch == 'c') {
					buffer[count++] = va_arg(paraList, char);
					if (count == MAX_BUFFER_SIZE) {
						syscall(SYS_WRITE, STD_OUT, (uint32_t)buffer, (uint32_t)count, 0, 0);
						count = 0;
					}
				}
				else {
					state = 2;
					return;
				}
				state = 0;
				break;
			case 2: return;
			default: break;
		}
	}
```

### sysprint

- 通过displayRow、displayCol两个变量维护光标位置
- 打印到显存，也就是将字符写入显存的地址
- 实现换行、清屏

```c
	for (i = 0; i < size; i++) {
		// 从 ES 段中以 str + i 作为偏移地址，读取 1 个字节，并将其存入 character。
		asm volatile("movb %%es:(%1), %0":"=r"(character):"r"(str+i));
		// TODO: 完成光标的维护和打印到显存
		if(character == '\n'){
			displayCol = 0;
			displayRow++;
		}
		else{
			print_out(displayRow, displayCol, character);
			displayCol++;
		}
		// 换行
		if(displayCol >= 80){
			displayRow++;
			displayCol = 0;
		}
		// 滚屏
		if(displayRow >= 25){
			scrollScreen();
			displayRow--;
			displayCol = 0;
		}
	}
```




## GetChar

getChar -> syscall -> irqHandle -> syscallHandle -> sysGetChar


getChar：系统调用读入字符
```c
char getChar(){ // 对应SYS_READ STD_IN
	char ret = 0;
	ret = (char)syscall(SYS_READ, STD_IN, 0, 0, 0, 0);
	return ret;
}
```

sysGetChar：
- 通过阻塞方式，等待输入结束
- 返回键盘缓冲区中的第一个字符
- 清除已读取的数据
```c
void sysGetChar(struct TrapFrame *tf){
	// TODO: 自由实现
 	wait_for_input();
	tf->eax = keyBuffer[bufferHead++];
	bufferHead = bufferTail;
}
```

## GetStr


GetStr：系统调用读入字符串
```c
void getStr(char *str, int size){ // 对应SYS_READ STD_STR
	syscall(SYS_READ, STD_STR, (uint32_t)str, (uint32_t)size, 0, 0);
	return;
}
```


sysGetStr：
- 通过阻塞方式，等待输入结束
- 读取键盘缓冲区到用户缓冲区，遇到换行符/到达最大读取字符数/到达缓冲区尾部停止
- 在用户缓冲区末尾加上 `\0` 
- 清除已读取的数据
```c
void sysGetStr(struct TrapFrame *tf){
	int sel = USEL(SEG_UDATA);
	char* str = (char*)tf->edx; // 用户缓冲区地址
	int size = tf->ebx; // 最大读取字符数
	asm volatile("movw %0, %%es"::"m"(sel));
 	wait_for_input();
	int i = 0;
	while(bufferHead < bufferTail && keyBuffer[bufferHead] != '\n' && i < size){
		if(keyBuffer[bufferHead] != 0) asm volatile("movl %0, %%es:(%1)"::"r"(keyBuffer[bufferHead]), "r"(str + i));
		i++;
		bufferHead++;
	}
	asm volatile("movb $0x00, %%es:(%0)"::"r"(str+size));
	bufferHead = bufferTail;
	tf->eax = size;
}
```


## now

调用链：now -> syscall -> int $0x80 -> irqHandle -> syscallHandle -> sysNow

使用RTC(real time clock)方式实现now函数：
- 通过系统调用，读取RTC寄存器
- 将bcd格式转换为数字格式

```c
void now(struct TimeInfo *tm_info) {
	tm_info->second = bcd_to_int(syscall(SYS_NOW, 0x00, 0, 0, 0, 0));
	tm_info->minute = bcd_to_int(syscall(SYS_NOW, 0x02, 0, 0, 0, 0));
	tm_info->hour = bcd_to_int(syscall(SYS_NOW, 0x04, 0, 0, 0, 0));
	tm_info->m_day = bcd_to_int(syscall(SYS_NOW, 0x07, 0, 0, 0, 0));
	tm_info->month = bcd_to_int(syscall(SYS_NOW, 0x08, 0, 0, 0, 0));
	tm_info->year = bcd_to_int(syscall(SYS_NOW, 0x09, 0, 0, 0, 0));
}
```

```c
#define SYS_NOW 2 
```

在syscallHandle中添加：
```c
  case 2: sysNow(tf);
```

sysNow:
- 通过直接访问RTC寄存器来获取当前的时间信息。
- 通过存放在ecx中的编号，读取对应的时间类型，返回时间

```c
void sysNow(struct TrapFrame *tf){
	int reg = tf->ecx;
	short port = 0x70;
	outByte(port, reg); // 向端口 0x70 写入偏移量
	port = 0x71;
	uint8_t data = inByte(port); // 从端口 0x71 读取数据
	tf->eax = data;
}
```


## sleep

在start.s中，仿照lab1，添加8253定时器设置，产生20 毫秒的时钟中断。


定时器中断：
- 中断号0x20
- 设置IDT，绑定0x20与对应的处理程序`irqTimer`。

```c
setIntr(idt + 0x20, SEG_KCODE, (uint32_t)irqTimer, DPL_KERN);
```
- 在irqHandle中添加：
```c
case 0x20: // irqTimer
		timerHandler(tf);break;
```

实现sleep库函数：
- 用一个标志timer_flag来表示是否处于sleep状态，用计数器timer_cnt来计时，到达时间后解除sleep状态
- `syscallHandle`中添加两个辅助函数sysSetTimeFlag、sysGetTimeFlag
- 在timerHandler中用计数器timer_cnt来计时


```c
void sleep(unsigned int seconds) {
    syscall(SYS_SET_TIME_FLAG, 0, 0, 0, 0, 0);
	uint32_t tm = 0;

	while(1){
		tm = syscall(SYS_GET_TIME_FLAG, 0, 0, 0, 0, 0);

		if(tm >= seconds * 50) {
			syscall(SYS_SET_TIME_FLAG, 0, 0, 0, 0, 0);
			return;
		}
	}
}

void timerHandler(struct TrapFrame *tf) {
	if(timer_flag == 1) timer_cnt++;
	else timer_cnt = 0;
}
void sysSetTimeFlag(struct TrapFrame *tf) {
	if(tf->ecx == 1) {
		timer_flag = 1;
		timer_cnt = 0;
	}
	else timer_flag = 0;
}

void sysGetTimeFlag(struct TrapFrame *tf) {
	tf->eax = timer_cnt;
}
```


# debug

在加载kernel、用户程序并进入用户空间时，出现花屏的错误，检查代码发现，是在初始化陷阱门时，没有将段描述符的索引值转换为实际的段选择子。
`ptr->segment = KSEL(selector);  `

tf的irq号为-1. 在irqHandle中添加`case -1: break;`




























lab2-STUID                     #自行修改后打包(.zip)提交
├── lab
│   ├── Makefile
│   ├── app                    #用户代码
│   │   ├── Makefile
**│   │   └── main.c             #主函数**
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
**│   │   │   │   ├── serial.h**
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