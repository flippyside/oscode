计算机启动

- 加电自检
- BIOS加载引导程序，放在内存`0x7c00`




To jump into 32-bit mode:

1. Disable interrupts
2. Load our GDT
3. Set a bit on the CPU control register `cr0`
4. Flush the CPU pipeline by issuing a carefully crafted far jump
5. Update all the segment registers
6. Update the stack
7. Call to a well-known label which contains the first useful code in 32 bits

进入保护模式
- 关中断
- 加载gdt
- 设置cr0.pe=1
- Flush the CPU pipeline by issuing a carefully crafted far jump
- 更新所有段寄存器
- 更新栈

