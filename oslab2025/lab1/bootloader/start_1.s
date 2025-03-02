/* Real Mode Hello World */
.code16

.extern bootMain

.global start   # 声明start符号是全局的
start: 
        # 初始化段寄存器
        movw %cs, %ax
        movw %ax, %ds
        movw %ax, %es
        movw %ax, %ss
        # 设置栈指针为0x7d00
        movw $0x7d00, %ax
        movw %ax, %sp 

        # TODO:通过中断输出Hello World，并且间隔1000ms打印新的一行

        # 将 timer_isr 的偏移地址存储到 0x70（偏移量）
        movw $timer_isr, 0x70          
        movw $0, 0x72 
        movw %bx, 0x0             # 向量表的低字节 (0x0)
        movw %bx, 0x2             # 向量表的高字节 (0x2)

        # 发送命令字节到控制寄存器端口0x43
        movw $0x36, %ax         # 方式3，用于定时产生中断（00110110b）
        movw $0x43, %dx
        out %al, %dx 

        # 计算计数值，产生20 毫秒的时钟中断，时钟频率为1193180 赫兹
        # 计数值 = (时钟频率 / 每秒中断次数) - 1
        #       = (1193180 / (1 / 0.02 )) - 1 = 23863
        # movw $23863, %ax
        movw $1193179, %ax

        # 将计数值分为低字节和高字节，发送到计数器0的数据端口（端口0x40）
        movw $0x40, %dx
        out %al, %dx 
        mov %ah, %al
        out %al, %dx
        sti                    # 启用中断
	
loop:
        jmp loop

message:
        .string "Hello, World!\n\0"

# 自定义ISR：
timer_isr:
        pusha                  # 保存所有寄存器的值
        # 使用BIOS中断 0x10 输出字符串
        
        movb $0x0E, %ah        # BIOS中断 0x10：写字符功能
        movb $'H', %al         # 打印H
        int $0x10              # 调用BIOS，中断输出字符

        movb $0x0E, %ah        
        movb $'e', %al         
        int $0x10              
        
        movb $0x0E, %ah        
        movb $'l', %al         
        int $0x10  

        movb $0x0E, %ah        
        movb $'l', %al         
        int $0x10  

        movb $0x0E, %ah        
        movb $'o', %al         
        int $0x10  

        movb $0x0E, %ah        
        movb $' ', %al         
        int $0x10  

        movb $0x0E, %ah        
        movb $'W', %al         
        int $0x10  

        movb $0x0E, %ah        
        movb $'o', %al         
        int $0x10  

        movb $0x0E, %ah        
        movb $'r', %al         
        int $0x10  

        movb $0x0E, %ah        
        movb $'l', %al         
        int $0x10  

        movb $0x0E, %ah        
        movb $'d', %al         
        int $0x10  
        
        movb $0x0E, %ah        
        movb $'\n', %al         
        int $0x10  

        # 结束后清除中断标志并返回
        iret                  


displayStr:
    pushw %bp
    movw 4(%esp), %ax
    movw %ax, %bp
    movw 6(%esp), %cx
    movw $0x1301, %ax
    movw $0x000c, %bx
    movw $0x0000, %dx
    int $0x10
    popw %bp
    ret

