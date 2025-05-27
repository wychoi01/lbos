global loader                          ; the entry symbol for ELF

extern kmain

extern kernel_virtual_end
extern kernel_virtual_start
extern kernel_physical_end
extern kernel_physical_start

extern init_paging
extern enable_paging
extern setup_higher_half

; Multiboot header constants
MBALIGN     equ 1<<0                   ; align loaded modules on page boundaries
MEMINFO     equ 1<<1                   ; provide memory map
FLAGS       equ MBALIGN | MEMINFO      ; this is the Multiboot 'flag' field
MAGIC       equ 0x1BADB002             ; define the magic number constant
CHECKSUM    equ -(MAGIC + FLAGS)

KERNEL_STACK_SIZE  equ 4096            ; size of stack in bytes
KERNEL_START_VADDR equ 0xC0000000      ; kernel starts at 3GB

section .data
align 4
grub_magic_number:
    dd 0
grub_multiboot_info:
    dd 0

section .bss
align 4
kernel_stack:
    resb KERNEL_STACK_SIZE

section .text
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

loader:
    mov ecx, grub_magic_number - KERNEL_START_VADDR
    mov [ecx], eax

    add ebx, KERNEL_START_VADDR
    mov ecx, grub_multiboot_info - KERNEL_START_VADDR
    mov [ecx], ebx

    call init_paging
    call enable_paging

    lea ecx, [higher_half]
    jmp ecx

higher_half:
    call setup_higher_half

    mov esp, kernel_stack + KERNEL_STACK_SIZE

    push kernel_virtual_end
    push kernel_virtual_start
    push kernel_physical_end
    push kernel_physical_start
    push DWORD [grub_magic_number]
    push DWORD [grub_multiboot_info]
    call kmain

.loop:
    jmp .loop
