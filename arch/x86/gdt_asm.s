global gdt_asm

extern gdt_ptr

; Constants from gdt.h
%define GDT_CODE_OFFSET 0x08
%define GDT_DATA_OFFSET 0x10

gdt_asm:
    lgdt [gdt_ptr]

    mov ax, GDT_DATA_OFFSET
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp GDT_CODE_OFFSET:.flush
.flush:
    ret
