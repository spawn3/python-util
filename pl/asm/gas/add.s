# First program for GAS, Clang/LLVM

.data
sum: .long 0

.text
.code32
.globl _start

_start:
    movl $25, %eax
    movl $50, %ebx
    addl %ebx, %eax
    movl %eax, sum

    pushl $0
    subl $4, %esp
    movl $1, %eax
    int $0x80
.end
