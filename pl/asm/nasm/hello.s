; First program for NASM

SECTION .data
num: dd 80

SECTION .bss
sum: resd 1

SECTION .text

global _main

_main:
    mov eax, DWORD [num]
    add eax, 20
    mov DWORD [sum], eax

    mov eax, 1
    mov ebx, 0
    int 80h
