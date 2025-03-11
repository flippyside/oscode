/* Real Mode Hello World */
.code16

.global _start

_start:
        # 初始化段寄存器
        movw %cs, %ax
        movw %ax, %ds
        movw %ax, %es
        movw %ax, %ss
        # 设置栈指针为0x7d00
        movw $0x7d00, %ax
        movw %ax, %sp 
    
        # 重新设置时钟中断处理程序
        mov $TIMER_ISR, 0x70
        movw $0, 0x72 
        movw %bx, 0x0             # 向量表的低字节 (0x0)
        movw %bx, 0x2             # 向量表的高字节 (0x2)
    
        # 设置8253定时器
        mov $0x36, %al  # 0x36：计数器0，模式3，低字节/高字节
        out %al, $0x43
        
        # 计算计数值，产生20 毫秒的时钟中断，时钟频率为1193180 赫兹
        # 计数值 = (时钟频率 / 每秒中断次数) - 1
        #       = (1193180 / (1 / 0.02 )) - 1 = 23863
        mov $23863, %ax
        out %al, $0x40  # 低字节
        mov %ah, %al
        out %al, $0x40  # 高字节
        
        sti  # 启用中断
    
.loop:
    jmp .loop

TIMER_ISR:
    pusha
    
    incw cnt  # 计时器计数+1
    
    cmpw $50, cnt  # 1000ms / 20ms = 50
    jne .end_itr
    
    movw $0, cnt  # 归零
    call print_message      # 调用打印函数

.end_itr:
    movb $0x20, %al  # 发送EOI（End of Interrupt）
    out %al, $0x20
    
    popa
    iret

print_message:
    pusha

    movb $0x0E, %ah  # BIOS teletype function
    mov $message, %si
.print_loop:
    lodsb
    testb %al, %al
    jz .done
    int $0x10  # BIOS 视频中断
    jmp .print_loop
.done:
    movb $0x0D, %al  # 回车
    int $0x10
    movb $0x0A, %al  # 换行
    int $0x10

    popa
    ret

message:
    .string "Hello, World!"

cnt:
    .word 0


