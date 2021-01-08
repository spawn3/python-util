all:

hello:
	nasm -f elf32 -o hello.o hello.s
	ld -e _main -melf_i386 -o hello hello.o
