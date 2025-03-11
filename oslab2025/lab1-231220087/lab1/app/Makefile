app.bin: app.s
	x86_64-elf-gcc -c -m32 app.s -o app.o
	x86_64-elf-ld -m elf_i386 -e start -Ttext 0x8c00 app.o -o app.elf
	x86_64-elf-objcopy -S -j .text -O binary app.elf app.bin

clean:
	rm -rf *.o *.elf *.bin
