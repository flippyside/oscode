#include "x86.h"
#include "device.h"
// 中断处理函数

extern int displayRow;
extern int displayCol;

extern uint32_t keyBuffer[MAX_KEYBUFFER_SIZE];
extern int bufferHead; // 指向下一个要读取的位置
extern int bufferTail; // 指向下一个要写入的位置

int timer_flag = 0;
uint32_t timer_cnt = 0;

int tail=0;

void GProtectFaultHandle(struct TrapFrame *tf);

void KeyboardHandle(struct TrapFrame *tf);

void timerHandler(struct TrapFrame *tf);
void syscallHandle(struct TrapFrame *tf);
void sysWrite(struct TrapFrame *tf);
void sysPrint(struct TrapFrame *tf);
void sysRead(struct TrapFrame *tf);
void sysGetChar(struct TrapFrame *tf);
void sysGetStr(struct TrapFrame *tf);
void sysSetTimeFlag(struct TrapFrame *tf);
void sysGetTimeFlag(struct TrapFrame *tf);

void sysNow(struct TrapFrame *tf);

void irqHandle(struct TrapFrame *tf) { // pointer tf = esp
	/*
	 * 中断处理程序
	 */
	/* Reassign segment register */
	asm volatile("movw %%ax, %%ds"::"a"(KSEL(SEG_KDATA)));

	// putNum(tf->irq);

	switch(tf->irq) {
		// TODO: 填好中断处理程序的调用
		case -1: {
			break;
		}
		case 0xd: // GProtectFault
		{
			GProtectFaultHandle(tf);
			break;
		}
		case 0x80: // irqSyscall
		{
			syscallHandle(tf);
			break;
		}
		case 0x21: // irqKeyboard
		{
			KeyboardHandle(tf);
			break;
		}
		case 0x20: // irqTimer
		{
			timerHandler(tf);
			break;
		}
		default:assert(0);
	}
}

void GProtectFaultHandle(struct TrapFrame *tf){
	char* str = "=== GProtectFault ===";
	putStr(str);
	assert(0);
	return;
}

void print_out(int displayRow, int displayCol, char character){
	int vga_mem = 0xB8000; // VGA 显存起始地址
	int color = 0x0c;
	// 每个字符占用两个字节（一个字节存储字符，另一个字节存储颜色信息）
	int pos = (displayRow * 80 + displayCol) * 2;
	*(char*)(vga_mem + pos) = character;
	*(char*)(vga_mem + pos + 1) = color;
}

void KeyboardHandle(struct TrapFrame *tf){
	uint32_t code = getKeyCode();
	int sel =  USEL(SEG_UDATA);

	if(code == 0xe){ // 退格符
		//要求只能退格用户键盘输入的字符串，且最多退到当行行首
		if(displayCol>0&&displayCol>tail){
			displayCol--;
			uint16_t data = 0 | (0x0c << 8);
			int pos = (80*displayRow+displayCol)*2;
			asm volatile("movw %0, (%1)"::"r"(data),"r"(pos+0xb8000));
		}
	}else if(code == 0x1c){ // 回车符
		//处理回车情况
		keyBuffer[bufferTail++]='\n';
		displayRow++;
		displayCol=0;
		tail=0;
		if(displayRow==25){
			scrollScreen();
			displayRow=24;
			displayCol=0;
		}
	}else if(code < 0x81){ 
		// TODO: 处理正常的字符
		char ch = getChar(code);
		if(ch >= 0x20){
			// putChar(ch);
			keyBuffer[bufferTail++] = ch;
			asm volatile("movw %0, %%es"::"m"(sel)); // 将 sel 的值加载到段寄存器 ES 中。
			print_out(displayRow, displayCol, ch);
			displayCol++;
			if(displayCol>=80){
				displayCol=0;
				displayRow++;
			}
			while(displayRow >= 25){
				scrollScreen();
				displayRow--;
				displayCol = 0;
			}
		}
	}
	updateCursor(displayRow, displayCol);

}


void timerHandler(struct TrapFrame *tf) {
	// TODO
	if(timer_flag == 1) timer_cnt++;
	else timer_cnt = 0;
}

void syscallHandle(struct TrapFrame *tf) {
	switch(tf->eax) { // syscall number
		case 0:
			sysWrite(tf);
			break; // for SYS_WRITE
		case 1:
			sysRead(tf);
			break; // for SYS_READ
		case 2:
			sysNow(tf);
			break; // for SYS_NOW
		case 3:
			sysSetTimeFlag(tf);
			break; // for SYS_SET_TIME_FLAG
		case 4:
			sysGetTimeFlag(tf);
			break; // for SYS_GET_TIME_FLAG

		default:break;
	}
}

void sysNow(struct TrapFrame *tf){
	int reg = tf->ecx;
	short port = 0x70;
	outByte(port, reg); // 向端口 0x70 写入偏移量
	port = 0x71;
	uint8_t data = inByte(port); // 从端口 0x71 读取数据
	tf->eax = data;
}

void sysWrite(struct TrapFrame *tf) {
	switch(tf->ecx) { // file descriptor
		case 0:
			sysPrint(tf);
			break; // for STD_OUT
		default:break;
	}
}

void sysPrint(struct TrapFrame *tf) {
	
	int sel =  USEL(SEG_UDATA);
	char *str = (char*)tf->edx;
	int size = tf->ebx;
	int i = 0;
	char character = 0;
	asm volatile("movw %0, %%es"::"m"(sel)); // 将 sel 的值加载到段寄存器 ES 中。
	
	for (i = 0; i < size; i++) {
		// 从 ES 段中以 str + i 作为偏移地址，读取 1 个字节，并将其存入 character。
		asm volatile("movb %%es:(%1), %0":"=r"(character):"r"(str+i)); // 将待print的字符串str的第i个字符赋值给character
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
	tail=displayCol;
	updateCursor(displayRow, displayCol);
}

void sysRead(struct TrapFrame *tf){
	switch(tf->ecx){ //file descriptor
		case 0:
			sysGetChar(tf);
			break; // for STD_IN
		case 1:
			sysGetStr(tf);
			break; // for STD_STR
		default:break;
	}
}

void wait_for_input(){
	asm volatile("sti");
	while(bufferHead == bufferTail || keyBuffer[bufferTail-1] != '\n')
		asm volatile("hlt");
	asm volatile("cli");
}

void sysGetChar(struct TrapFrame *tf){
	// TODO: 自由实现
 	wait_for_input();
	tf->eax = keyBuffer[bufferHead++];
	bufferHead = bufferTail;
}

void sysGetStr(struct TrapFrame *tf){
	// TODO: 自由实现
	int sel = USEL(SEG_UDATA);
	char* str = (char*)tf->edx;
	int size = tf->ebx;

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




void sysSetTimeFlag(struct TrapFrame *tf) {
	// TODO: 自由实现
	char* str = "from sysSetTimeFlag\n";
	putStr(str);
	putNum((int)(tf->ecx));
	char* str1 = "\nTimeFlag\n";
	putStr(str1);

	if(tf->ecx == 1) {
		timer_flag = 1;
		timer_cnt = 0;
	}
	else timer_flag = 0;
}

void sysGetTimeFlag(struct TrapFrame *tf) {
	// TODO: 自由实现
	// char* str = "\n";
	// putNum(timer_cnt);
	// putStr(str);
	;
	tf->eax = timer_cnt;
}
