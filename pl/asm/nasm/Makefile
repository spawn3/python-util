all:

hello:
	nasm -f elf32 -o hello.o hello.s
	ld -e _main -melf_i386 -o hello hello.o

eatsyscall:
	nasm -f elf32 -o eatsyscall.o eatsyscall.s
	ld -e _start -melf_i386 -o eatsyscall eatsyscall.o
