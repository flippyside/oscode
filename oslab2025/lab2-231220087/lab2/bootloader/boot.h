#ifndef BOOT_H
#define BOOT_H

struct ELFHeader {
	unsigned int   magic;
	unsigned char  elf[12];
	unsigned short type;
	unsigned short machine;
	unsigned int   version;
	unsigned int   entry;
	unsigned int   phoff;
	unsigned int   shoff;
	unsigned int   flags;
	unsigned short ehsize;
	unsigned short phentsize;
	unsigned short phnum;
	unsigned short shentsize;
	unsigned short shnum;
	unsigned short shstrndx;
};

/* ELF32 Program header */
struct ProgramHeader {
	unsigned int type;
	unsigned int off;
	unsigned int vaddr;
	unsigned int paddr;
	unsigned int filesz;
	unsigned int memsz;
	unsigned int flags;
	unsigned int align;
};

typedef struct ELFHeader ELFHeader;//typedef ,discard "struct"
typedef struct ProgramHeader ProgramHeader;

void waitDisk(void);

void readSect(void *dst, int offset);

/* I/O functions */
/*
"a" (data)：输入约束，表示将变量 data 的值加载到 eax 寄存器
"d" (port)：输入约束，表示将变量 port 的值加载到 edx 寄存器
*/
static inline char inByte(short port) { // 从 I/O 端口 port 读取数据并存储到 data 变量中
	char data;
	asm volatile("in %1,%0" : "=a" (data) : "d" (port));
	return data;
}

static inline int inLong(short port) {
	int data;
	asm volatile("in %1, %0" : "=a" (data) : "d" (port));
	return data;
}

static inline void outByte(short port, char data) { // 将 data 变量中的数据写入到 I/O 端口 port
	asm volatile("out %0,%1" : : "a" (data), "d" (port));
}

#endif
