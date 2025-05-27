section .text
global switch_to_process

; void switch_to_process(process_t* next)
switch_to_process:
    mov     eax, [esp + 4]

    ; Load cr3
    mov     ecx, [eax + 0x3C]
    mov     cr3, ecx

    mov     esp, [eax + 0x24]

    ; Restore registers
    mov     edi, [eax + 0x08]
    mov     esi, [eax + 0x0C]
    mov     ebp, [eax + 0x10]
    mov     edx, [eax + 0x14]
    mov     ecx, [eax + 0x18]
    mov     ebx, [eax + 0x1C]
    mov     eax, [eax + 0x20]

    ; Return into process
    iret
