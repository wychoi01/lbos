extern interrupt_handler
extern kernel_stack

global interrupt_sti
global interrupt_cli

%macro NO_ERROR_HANDLER 1
global interrupt_handler_%1
interrupt_handler_%1:
    push    dword 0
    push    dword %1
    jmp     common_interrupt_handler
%endmacro

%macro ERROR_HANDLER 1
global interrupt_handler_%1
interrupt_handler_%1:
    push    dword %1
    jmp     common_interrupt_handler
%endmacro

section .text:

common_interrupt_handler:
    push    esp
    add     DWORD [esp], 8
    push    eax
    push    ebx
    push    ecx
    push    edx
    push    ebp
    push    esi
    push    edi
    call    interrupt_handler
    pop     edi
    pop     esi
    pop     ebp
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax
    pop     esp
    iret

interrupt_sti:
    sti
    ret

interrupt_cli:
    cli
    ret

; protected mode exceptions
NO_ERROR_HANDLER 0
NO_ERROR_HANDLER 1
NO_ERROR_HANDLER 2
NO_ERROR_HANDLER 3
NO_ERROR_HANDLER 4
NO_ERROR_HANDLER 5
NO_ERROR_HANDLER 6
NO_ERROR_HANDLER 7
ERROR_HANDLER 8
NO_ERROR_HANDLER 9
ERROR_HANDLER 10
ERROR_HANDLER 11
ERROR_HANDLER 12
ERROR_HANDLER 13
ERROR_HANDLER 14
NO_ERROR_HANDLER 15
NO_ERROR_HANDLER 16
ERROR_HANDLER 17
NO_ERROR_HANDLER 18
NO_ERROR_HANDLER 19

; reserved
NO_ERROR_HANDLER 20
NO_ERROR_HANDLER 21
NO_ERROR_HANDLER 22
NO_ERROR_HANDLER 23
NO_ERROR_HANDLER 24
NO_ERROR_HANDLER 25
NO_ERROR_HANDLER 26
NO_ERROR_HANDLER 27
NO_ERROR_HANDLER 28
NO_ERROR_HANDLER 29
ERROR_HANDLER 30
NO_ERROR_HANDLER 31

; irqs
NO_ERROR_HANDLER 32
NO_ERROR_HANDLER 33
NO_ERROR_HANDLER 34
NO_ERROR_HANDLER 35
NO_ERROR_HANDLER 36
NO_ERROR_HANDLER 37
NO_ERROR_HANDLER 38
NO_ERROR_HANDLER 39
NO_ERROR_HANDLER 40
NO_ERROR_HANDLER 41
NO_ERROR_HANDLER 42
NO_ERROR_HANDLER 43
NO_ERROR_HANDLER 44
NO_ERROR_HANDLER 45
NO_ERROR_HANDLER 46
NO_ERROR_HANDLER 47

; syscall interrupt (0x80 = 128)
NO_ERROR_HANDLER 128
