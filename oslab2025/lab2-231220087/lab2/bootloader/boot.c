#include "boot.h"

#define SECTSIZE 512


void bootMain(void) {
	int i = 0;
	int phoff = 0x34; // program header offset， 程序头表 在 ELF 文件中的偏移量
	int offset = 0x1000; // .text section offset，.text 段在 ELF 文件中的偏移量, 通常为0x1000
	unsigned int elf = 0x100000; // physical memory addr to load，ELF 加载到物理内存的起始地址
	void (*kMainEntry)(void); // 函数指针
	kMainEntry = (void(*)(void))0x100000; // entry address of the program 程序入口点

	for (i = 0; i < 200; i++) { // 加载elf
		readSect((void*)(elf + i*512), 1+i);
	}

	// TODO: 阅读boot.h查看elf相关信息，填写kMainEntry、phoff、offset

	struct ELFHeader* elfHeader = (void*)elf;
	kMainEntry = (void(*)(void))elfHeader->entry; // 指向 ELF 文件的入口点
	phoff = elfHeader->phoff;

	for (i = 0; i < 200 * 512; i++) {
			*(unsigned char *)(elf + i) = *(unsigned char *)(elf + i + offset);
	}

	kMainEntry();
}

	// ProgramHeader* programHeader = (ProgramHeader*)(elf + phoff); // 程序头表
	// offset = programHeader->off;

void waitDisk(void) { // waiting for disk
	while((inByte(0x1F7) & 0xC0) != 0x40);
}

void readSect(void *dst, int offset) { // reading a sector of disk。offset：扇区编号
	// 硬盘使用 28 位寻址
	int i;
	waitDisk();
	outByte(0x1F2, 1); // 向扇区计数寄存器写入1，请求读取一个扇区
	// 设置了要读取的扇区的偏移量（offset）
	outByte(0x1F3, offset);
	outByte(0x1F4, offset >> 8);
	outByte(0x1F5, offset >> 16);
	outByte(0x1F6, (offset >> 24) | 0xE0);
	outByte(0x1F7, 0x20);

	waitDisk();
	for (i = 0; i < SECTSIZE / 4; i ++) {
		// 0x1F0: 硬盘的数据寄存器
		((int *)dst)[i] = inLong(0x1F0); // 每次读取 4 字节，存储到dst中
	}
}
