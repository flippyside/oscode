# TODO: This is lab1.2
/* Protected Mode Hello World */
.code16

.global start
start:
	movw %cs, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %ss
	
	# TODO:关闭中断
	cli

	# 启动A20总线
	inb $0x92, %al 
	orb $0x02, %al
	outb %al, $0x92

	# 加载GDTR
	data32 addr32 lgdt gdtDesc # loading gdtr, data32, addr32

	# TODO：设置CR0的PE位（第0位）为1
	movl %cr0, %eax
	orl $0x1, %eax
	movl %eax, %cr0

	# 长跳转切换至保护模式
	data32 ljmp $0x08, $start32 # reload code segment selector and ljmp to start32, data32

.code32
start32:
	
	movw $0x10, %ax # setting data segment selector
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %ss
	movw $0x18, %ax # setting graphics data segment selector
	movw %ax, %gs
	
	movl $0x8000, %eax # setting esp
	movl %eax, %esp

	pushl $13 # pushing the size to print into stack
	pushl $message # pushing the address of message into stack
	calll displayStr # calling the display function

loop32:
	jmp loop32

message:
	.string "Hello, World!\n\0"


displayStr:
	movl 4(%esp), %ebx
	movl 8(%esp), %ecx
	movl $((80*5+0)*2), %edi
	movb $0x0c, %ah
nextChar:
	movb (%ebx), %al
	movw %ax, %gs:(%edi)
	addl $2, %edi
	incl %ebx
	loopnz nextChar # loopnz decrease ecx by 1
	ret


.p2align 2
gdt: # 8 bytes for each table entry, at least 1 entry
	# .word limit[15:0],base[15:0]
	# .byte base[23:16],(0x90|(type)),(0xc0|(limit[19:16])),base[31:24]
	# GDT第一个表项为空
	.word 0,0
	.byte 0,0,0,0

	# TODO：code segment entry （0x08）
	.word 0xffff 	# limit 最大 4GB
	.word 0			# base
	.byte 0			# base
	.byte 0x9a		# attributes: P=1, DPL=00, S=1, Type= 1010 -> 1001 1010
	.byte 0xcf		# attributes: G=1, D=1, AVL=00, limit=1111 -> 1100 1111
	.byte 0			# base

	# TODO：data segment entry （0x10）
    .word 0xFFFF
    .word 0
    .byte 0
    .byte 0x92  # P=1, DPL=0, S=1, Type=0010 (RW)
    .byte 0xCF  # G=1, D=1, Limit=1111
    .byte 0

	# TODO：graphics segment entry （0x18）
    .word 0xFFFF
    .word 0x8000
    .byte 0x0B
    .byte 0x92  # P=1, DPL=0, S=1, Type=0010 (RW)
    .byte 0xCF  # G=1, D=1, Limit=1111
    .byte 0


gdtDesc: 
	.word (gdtDesc - gdt -1) 
	.long gdt 