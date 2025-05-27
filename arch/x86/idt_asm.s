global load_idt

section .text

; Load the IDT
; void load_idt(struct idt_ptr* idt_ptr)
load_idt:
    mov eax, [esp+4]
    lidt [eax]
    ret
