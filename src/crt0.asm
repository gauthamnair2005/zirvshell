[bits 64]
section .text
global _start
extern main

_start:
    ; The kernel should have set up a stack for us
    ; and passed any arguments in rdi, rsi, etc.
    
    call main

    ; If main returns, we exit
    mov rax, 60         ; SYS_EXIT
    xor rdi, rdi        ; status 0
    syscall

    ; Should never reach here
.halt:
    hlt
    jmp .halt
