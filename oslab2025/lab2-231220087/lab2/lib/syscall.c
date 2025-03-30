#include "lib.h"
#include "types.h"

// 系统调用

/*
va_list 指向参数列表的指针
将所有大小向上对齐到4的倍数
设置va_list指针指向第一个可变参数的位置
并将指针移动到下一个参数
将va_list指针置空
*/

typedef char *  va_list;
#define _INTSIZEOF(n)   ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )
#define va_start(ap, format) ( ap = (va_list)&format+ _INTSIZEOF(format) )
#define va_arg(ap, type) ( *(type*)((ap += _INTSIZEOF(type)) -_INTSIZEOF(type)) )
#define va_end(ap)  ( ap = (va_list)0 )

/*
 * io lib here
 * 库函数写在这
 */


//static inline int32_t syscall(int num, uint32_t a1,uint32_t a2,
int32_t syscall(int num, uint32_t a1,uint32_t a2,
		uint32_t a3, uint32_t a4, uint32_t a5)
{
	int32_t ret = 0;
	//Generic system call: pass system call number in AX
	//up to five parameters in DX,CX,BX,DI,SI
	//Interrupt kernel with T_SYSCALL
	//
	//The "volatile" tells the assembler not to optimize
	//this instruction away just because we don't use the
	//return value
	//
	//The last clause tells the assembler that this can potentially
	//change the condition and arbitrary memory locations.

	/*
		 Note: ebp shouldn't be flushed
	    May not be necessary to store the value of eax, ebx, ecx, edx, esi, edi
	*/
	uint32_t eax, ecx, edx, ebx, esi, edi;
	uint16_t selector;
	
	asm volatile("movl %%eax, %0":"=m"(eax));
	asm volatile("movl %%ecx, %0":"=m"(ecx));
	asm volatile("movl %%edx, %0":"=m"(edx));
	asm volatile("movl %%ebx, %0":"=m"(ebx));
	asm volatile("movl %%esi,  %0":"=m"(esi));
	asm volatile("movl %%edi,  %0":"=m"(edi));
	asm volatile("movl %0, %%eax": :"m"(num));
	asm volatile("movl %0, %%ecx"::"m"(a1));
	asm volatile("movl %0, %%edx"::"m"(a2));
	asm volatile("movl %0, %%ebx"::"m"(a3));
	asm volatile("movl %0, %%esi" ::"m"(a4));
	asm volatile("movl %0, %%edi" ::"m"(a5));
	asm volatile("int $0x80");
	asm volatile("movl %%eax, %0":"=m"(ret));
	asm volatile("movl %0, %%eax"::"m"(eax));
	asm volatile("movl %0, %%ecx"::"m"(ecx));
	asm volatile("movl %0, %%edx"::"m"(edx));
	asm volatile("movl %0, %%ebx"::"m"(ebx));
	asm volatile("movl %0, %%esi"::"m"(esi));
	asm volatile("movl %0, %%edi"::"m"(edi));
	
	asm volatile("movw %%ss, %0":"=m"(selector)); //%ds is reset after iret
	//selector = 16;
	asm volatile("movw %%ax, %%ds"::"a"(selector));
	
	return ret;
}

char getChar(){ // 对应SYS_READ STD_IN
	// TODO: 实现getChar函数，方式不限
	char ret = 0;
	ret = (char)syscall(SYS_READ, STD_IN, 0, 0, 0, 0);
	return ret;
}

void getStr(char *str, int size){ // 对应SYS_READ STD_STR
	// TODO: 实现getStr函数，方式不限
	syscall(SYS_READ, STD_STR, (uint32_t)str, (uint32_t)size, 0, 0);
	return;
}

int dec2Str(int decimal, char *buffer, int size, int count);
int hex2Str(uint32_t hexadecimal, char *buffer, int size, int count);
int str2Str(char *string, char *buffer, int size, int count);

void printf(const char *format,...){
	int i=0; // format index
	char buffer[MAX_BUFFER_SIZE];
	int count=0; // buffer index
	void *paraList=(void*)&format; // address of format in stack
	int state=0; // 0: legal character; 1: '%'; 2: illegal format

	va_start(paraList, format); // 设置va_list指针指向第一个可变参数的位置

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
	
	if(count!=0)
		syscall(SYS_WRITE, STD_OUT, (uint32_t)buffer, (uint32_t)count, 0, 0);

}

void sleep(unsigned int seconds) {
	// TODO: 实现一个阻塞方式的sleep库函数
    syscall(SYS_SET_TIME_FLAG, 0, 0, 0, 0, 0);
    syscall(SYS_SET_TIME_FLAG, 1, 0, 0, 0, 0);
	uint32_t tm = 0;

    // syscall(SYS_WRITE, STD_OUT, (uint32_t)((char *)"In sleep\n"), sizeof("In sleep\n"), 0, 0);

	while(1){
		tm = syscall(SYS_GET_TIME_FLAG, 0, 0, 0, 0, 0);

		if(tm >= seconds * 50) {
			syscall(SYS_SET_TIME_FLAG, 0, 0, 0, 0, 0);
			return;
		}
	}
}

int bcd_to_int(uint8_t bcd) {
    return (bcd & 0x0F) + ((bcd >> 4) * 10);
}

void now(struct TimeInfo *tm_info) {
	// TODO: 实现now函数，使用RTC方式
	
	tm_info->second = bcd_to_int(syscall(SYS_NOW, 0x00, 0, 0, 0, 0));
	tm_info->minute = bcd_to_int(syscall(SYS_NOW, 0x02, 0, 0, 0, 0));
	tm_info->hour = bcd_to_int(syscall(SYS_NOW, 0x04, 0, 0, 0, 0));
	tm_info->m_day = bcd_to_int(syscall(SYS_NOW, 0x07, 0, 0, 0, 0));
	tm_info->month = bcd_to_int(syscall(SYS_NOW, 0x08, 0, 0, 0, 0));
	tm_info->year = bcd_to_int(syscall(SYS_NOW, 0x09, 0, 0, 0, 0));

	// syscall(SYS_WRITE, STD_OUT, (uint32_t)((char *)"In now\n"), sizeof("In now\n"), 0, 0);
}


int dec2Str(int decimal, char *buffer, int size, int count) {// 十进制整数转换为字符串 返回更新后的count值
	int i=0;
	int temp;
	int number[16];

	if(decimal<0){
		buffer[count]='-';
		count++;
		if(count==size) {
			syscall(SYS_WRITE, STD_OUT, (uint32_t)buffer, (uint32_t)size, 0, 0);
			count=0;
		}
		temp=decimal/10;
		number[i]=temp*10-decimal;
		decimal=temp;
		i++;
		while(decimal!=0){
			temp=decimal/10;
			number[i]=temp*10-decimal;
			decimal=temp;
			i++;
		}
	}
	else{
		temp=decimal/10;
		number[i]=decimal-temp*10;
		decimal=temp;
		i++;
		while(decimal!=0){
			temp=decimal/10;
			number[i]=decimal-temp*10;
			decimal=temp;
			i++;
		}
	}

	while(i!=0){
		buffer[count]=number[i-1]+'0';
		count++;
		if(count==size) {
			syscall(SYS_WRITE, STD_OUT, (uint32_t)buffer, (uint32_t)size, 0, 0);
			count=0;
		}
		i--;
	}
	return count;
}

int hex2Str(uint32_t hexadecimal, char *buffer, int size, int count) { // 十六进制整数转换为字符串 返回更新后的count值
	int i=0;
	uint32_t temp=0;
	int number[16];

	temp=hexadecimal>>4;
	number[i]=hexadecimal-(temp<<4);
	hexadecimal=temp;
	i++;
	while(hexadecimal!=0){
		temp=hexadecimal>>4;
		number[i]=hexadecimal-(temp<<4);
		hexadecimal=temp;
		i++;
	}

	while(i!=0){
		if(number[i-1]<10)
			buffer[count]=number[i-1]+'0';
		else
			buffer[count]=number[i-1]-10+'a';
		count++;
		if(count==size) {
			syscall(SYS_WRITE, STD_OUT, (uint32_t)buffer, (uint32_t)size, 0, 0);
			count=0;
		}
		i--;
	}
	return count;
}

int str2Str(char *string, char *buffer, int size, int count) {
	int i=0;
	while(string[i]!=0){
		buffer[count]=string[i];
		count++;
		if(count==size) {
			syscall(SYS_WRITE, STD_OUT, (uint32_t)buffer, (uint32_t)size, 0, 0);
			count=0;
		}
		i++;
	}
	return count;
}

