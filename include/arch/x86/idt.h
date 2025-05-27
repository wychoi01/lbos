#ifndef IDT_H
#define IDT_H

#include <stdint.h>

#define IDT_NUM_ENTRIES 256

#define SYSCALL_INT_IDX 0x80

#define PL0 0x00
#define PL3 0x03

#define SEGSEL_KERNEL_CS 0x08
#define SEGSEL_KERNEL_DS 0x10
#define SEGSEL_USER_CS 0x18
#define SEGSEL_USER_DS 0x20

struct idt_gate {
  uint16_t handler_low;
  uint16_t segsel;
  uint8_t zero;
  uint8_t config;
  uint16_t handler_high;
} __attribute__((packed));
typedef struct idt_gate idt_gate_t;

struct idt_ptr {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed));
typedef struct idt_ptr idt_ptr_t;

#define IDT_FLAG_PRESENT 0x80
#define IDT_FLAG_RING0 0x00
#define IDT_FLAG_RING3 0x60
#define IDT_FLAG_32BIT 0x08
#define IDT_FLAG_TRAP 0x0F
#define IDT_FLAG_INTERRUPT 0x0E

void idt_init(void);
void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags);
void idt_load_and_set(uint32_t idt_ptr);
void load_idt(uint32_t idt_ptr);

#define INTERRUPT_DIVIDE_BY_ZERO 0
#define INTERRUPT_DEBUG 1
#define INTERRUPT_NMI 2
#define INTERRUPT_BREAKPOINT 3
#define INTERRUPT_OVERFLOW 4
#define INTERRUPT_BOUND_RANGE_EXCEEDED 5
#define INTERRUPT_INVALID_OPCODE 6
#define INTERRUPT_DEVICE_NOT_AVAILABLE 7
#define INTERRUPT_DOUBLE_FAULT 8
#define INTERRUPT_COPROCESSOR_SEGMENT_OVERRUN 9
#define INTERRUPT_INVALID_TSS 10
#define INTERRUPT_SEGMENT_NOT_PRESENT 11
#define INTERRUPT_STACK_SEGMENT_FAULT 12
#define INTERRUPT_GENERAL_PROTECTION_FAULT 13
#define INTERRUPT_PAGE_FAULT 14
#define INTERRUPT_RESERVED 15
#define INTERRUPT_X87_FLOATING_POINT 16
#define INTERRUPT_ALIGNMENT_CHECK 17
#define INTERRUPT_MACHINE_CHECK 18
#define INTERRUPT_SIMD_FLOATING_POINT 19
#define INTERRUPT_VIRTUALIZATION 20
#define INTERRUPT_CONTROL_PROTECTION 21
#define INTERRUPT_RESERVED_22 22
#define INTERRUPT_RESERVED_23 23
#define INTERRUPT_RESERVED_24 24
#define INTERRUPT_RESERVED_25 25
#define INTERRUPT_RESERVED_26 26
#define INTERRUPT_RESERVED_27 27
#define INTERRUPT_RESERVED_28 28
#define INTERRUPT_RESERVED_29 29
#define INTERRUPT_RESERVED_30 30
#define INTERRUPT_RESERVED_31 31
#define IRQ_PIT 32
#define IRQ_KEYBOARD 33
#define IRQ_CASCADE 34
#define IRQ_COM2 35
#define IRQ_COM1 36
#define IRQ_LPT2 37
#define IRQ_FLOPPY 38
#define IRQ_LPT1 39
#define IRQ_CMOS 40
#define IRQ_FREE_41 41
#define IRQ_FREE_42 42
#define IRQ_FREE_43 43
#define IRQ_PS2_MOUSE 44
#define IRQ_FPU 45
#define IRQ_PRIMARY_ATA 46
#define IRQ_SECONDARY_ATA 47

extern void interrupt0();  /* Divide by zero */
extern void interrupt1();  /* Debug */
extern void interrupt2();  /* Non-maskable interrupt */
extern void interrupt3();  /* Breakpoint */
extern void interrupt4();  /* Overflow */
extern void interrupt5();  /* Bound range exceeded */
extern void interrupt6();  /* Invalid opcode */
extern void interrupt7();  /* Device not available */
extern void interrupt8();  /* Double fault */
extern void interrupt9();  /* Coprocessor segment overrun */
extern void interrupt10(); /* Invalid TSS */
extern void interrupt11(); /* Segment not present */
extern void interrupt12(); /* Stack segment fault */
extern void interrupt13(); /* General protection fault */
extern void interrupt14(); /* Page fault */
extern void interrupt15(); /* Reserved */
extern void interrupt16(); /* x87 floating-point exception */
extern void interrupt17(); /* Alignment check */
extern void interrupt18(); /* Machine check */
extern void interrupt19(); /* SIMD floating-point exception */
extern void interrupt20(); /* Virtualization exception */
extern void interrupt21(); /* Reserved */
extern void interrupt22(); /* Reserved */
extern void interrupt23(); /* Reserved */
extern void interrupt24(); /* Reserved */
extern void interrupt25(); /* Reserved */
extern void interrupt26(); /* Reserved */
extern void interrupt27(); /* Reserved */
extern void interrupt28(); /* Reserved */
extern void interrupt29(); /* Reserved */
extern void interrupt30(); /* Security exception */
extern void interrupt31(); /* Reserved */
extern void interrupt32(); /* IRQ 0 (PIT) */
extern void interrupt33(); /* IRQ 1 (Keyboard) */
extern void interrupt34(); /* IRQ 2 (Cascade) */
extern void interrupt35(); /* IRQ 3 (COM2) */
extern void interrupt36(); /* IRQ 4 (COM1) */
extern void interrupt37(); /* IRQ 5 (LPT2) */
extern void interrupt38(); /* IRQ 6 (Floppy) */
extern void interrupt39(); /* IRQ 7 (LPT1) */
extern void interrupt40(); /* IRQ 8 (CMOS) */
extern void interrupt41(); /* IRQ 9 (Free) */
extern void interrupt42(); /* IRQ 10 (Free) */
extern void interrupt43(); /* IRQ 11 (Free) */
extern void interrupt44(); /* IRQ 12 (PS/2 Mouse) */
extern void interrupt45(); /* IRQ 13 (FPU) */
extern void interrupt46(); /* IRQ 14 (Primary ATA) */
extern void interrupt47(); /* IRQ 15 (Secondary ATA) */

#endif /* IDT_H */
