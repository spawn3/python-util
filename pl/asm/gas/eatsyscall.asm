.section .data

EatMsg: db "Eat at Joe's!", 10
EatLen: equ $-EatMsg

.section .bss
.section .text

global _start

_start:
    nop
    mov eax, 4
    mov ebx, 1
    mov ecx, EatMsg
    mov edx, EatLen
    int 80H

    mov eax, 1
    mov ebx, 0
    int 80H
