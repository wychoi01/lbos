#include "arch/x86/idt.h"
#include "arch/x86/gdt.h"
#include "arch/x86/interrupt.h"
#include "lib/log.h"

#define IDT_TRAP_GATE_TYPE 1
#define CREATE_IDT_GATE(idx) create_idt_gate(idx, (uint32_t)&interrupt_handler_##idx, IDT_TRAP_GATE_TYPE, PL0);
#define DECLARE_INTERRUPT_HANDLER(i) void interrupt_handler_##i(void)

/* Protected mode exceptions interrupts */
DECLARE_INTERRUPT_HANDLER(0);
DECLARE_INTERRUPT_HANDLER(1);
DECLARE_INTERRUPT_HANDLER(2);
DECLARE_INTERRUPT_HANDLER(3);
DECLARE_INTERRUPT_HANDLER(4);
DECLARE_INTERRUPT_HANDLER(5);
DECLARE_INTERRUPT_HANDLER(6);
DECLARE_INTERRUPT_HANDLER(7);
DECLARE_INTERRUPT_HANDLER(8);
DECLARE_INTERRUPT_HANDLER(9);
DECLARE_INTERRUPT_HANDLER(10);
DECLARE_INTERRUPT_HANDLER(11);
DECLARE_INTERRUPT_HANDLER(12);
DECLARE_INTERRUPT_HANDLER(13);
DECLARE_INTERRUPT_HANDLER(14);
DECLARE_INTERRUPT_HANDLER(15);
DECLARE_INTERRUPT_HANDLER(16);
DECLARE_INTERRUPT_HANDLER(17);
DECLARE_INTERRUPT_HANDLER(18);
DECLARE_INTERRUPT_HANDLER(19);

/* Additional interrupts */
DECLARE_INTERRUPT_HANDLER(20);
DECLARE_INTERRUPT_HANDLER(21);
DECLARE_INTERRUPT_HANDLER(22);
DECLARE_INTERRUPT_HANDLER(23);
DECLARE_INTERRUPT_HANDLER(24);
DECLARE_INTERRUPT_HANDLER(25);
DECLARE_INTERRUPT_HANDLER(26);
DECLARE_INTERRUPT_HANDLER(27);
DECLARE_INTERRUPT_HANDLER(28);
DECLARE_INTERRUPT_HANDLER(29);
DECLARE_INTERRUPT_HANDLER(30);
DECLARE_INTERRUPT_HANDLER(31);

/* IRQs */
DECLARE_INTERRUPT_HANDLER(32);
DECLARE_INTERRUPT_HANDLER(33);
DECLARE_INTERRUPT_HANDLER(34);
DECLARE_INTERRUPT_HANDLER(35);
DECLARE_INTERRUPT_HANDLER(36);
DECLARE_INTERRUPT_HANDLER(37);
DECLARE_INTERRUPT_HANDLER(38);
DECLARE_INTERRUPT_HANDLER(39);
DECLARE_INTERRUPT_HANDLER(40);
DECLARE_INTERRUPT_HANDLER(41);
DECLARE_INTERRUPT_HANDLER(42);
DECLARE_INTERRUPT_HANDLER(43);
DECLARE_INTERRUPT_HANDLER(44);
DECLARE_INTERRUPT_HANDLER(45);
DECLARE_INTERRUPT_HANDLER(46);
DECLARE_INTERRUPT_HANDLER(47);

/* System call */
DECLARE_INTERRUPT_HANDLER(128);

idt_gate_t idt[IDT_NUM_ENTRIES];

static void create_idt_gate(uint8_t n, uint32_t handler, uint8_t type, uint8_t pl) {
  switch (n) {
  case INTERRUPT_DIVIDE_BY_ZERO:
    LOG_DEBUG("\tCreating IDT gate: Divide by Zero Exception (%d)", INTERRUPT_DIVIDE_BY_ZERO);
    break;
  case INTERRUPT_DEBUG:
    LOG_DEBUG("\tCreating IDT gate: Debug Exception (%d)", INTERRUPT_DEBUG);
    break;
  case INTERRUPT_NMI:
    LOG_DEBUG("\tCreating IDT gate: Non-Maskable Interrupt (%d)", INTERRUPT_NMI);
    break;
  case INTERRUPT_BREAKPOINT:
    LOG_DEBUG("\tCreating IDT gate: Breakpoint Exception (%d)", INTERRUPT_BREAKPOINT);
    break;
  case INTERRUPT_OVERFLOW:
    LOG_DEBUG("\tCreating IDT gate: Overflow Exception (%d)", INTERRUPT_OVERFLOW);
    break;
  case INTERRUPT_BOUND_RANGE_EXCEEDED:
    LOG_DEBUG("\tCreating IDT gate: Bound Range Exceeded Exception (%d)", INTERRUPT_BOUND_RANGE_EXCEEDED);
    break;
  case INTERRUPT_INVALID_OPCODE:
    LOG_DEBUG("\tCreating IDT gate: Invalid Opcode Exception (%d)", INTERRUPT_INVALID_OPCODE);
    break;
  case INTERRUPT_DEVICE_NOT_AVAILABLE:
    LOG_DEBUG("\tCreating IDT gate: Device Not Available Exception (%d)", INTERRUPT_DEVICE_NOT_AVAILABLE);
    break;
  case INTERRUPT_DOUBLE_FAULT:
    LOG_DEBUG("\tCreating IDT gate: Double Fault Exception (%d)", INTERRUPT_DOUBLE_FAULT);
    break;
  case INTERRUPT_COPROCESSOR_SEGMENT_OVERRUN:
    LOG_DEBUG("\tCreating IDT gate: Coprocessor Segment Overrun (%d)", INTERRUPT_COPROCESSOR_SEGMENT_OVERRUN);
    break;
  case INTERRUPT_INVALID_TSS:
    LOG_DEBUG("\tCreating IDT gate: Invalid TSS Exception (%d)", INTERRUPT_INVALID_TSS);
    break;
  case INTERRUPT_SEGMENT_NOT_PRESENT:
    LOG_DEBUG("\tCreating IDT gate: Segment Not Present Exception (%d)", INTERRUPT_SEGMENT_NOT_PRESENT);
    break;
  case INTERRUPT_STACK_SEGMENT_FAULT:
    LOG_DEBUG("\tCreating IDT gate: Stack Segment Fault Exception (%d)", INTERRUPT_STACK_SEGMENT_FAULT);
    break;
  case INTERRUPT_GENERAL_PROTECTION_FAULT:
    LOG_DEBUG("\tCreating IDT gate: General Protection Fault Exception (%d)", INTERRUPT_GENERAL_PROTECTION_FAULT);
    break;
  case INTERRUPT_PAGE_FAULT:
    LOG_DEBUG("\tCreating IDT gate: Page Fault Exception (%d)", INTERRUPT_PAGE_FAULT);
    break;
  case INTERRUPT_RESERVED:
    LOG_DEBUG("\tCreating IDT gate: Reserved Exception (%d)", INTERRUPT_RESERVED);
    break;
  case INTERRUPT_X87_FLOATING_POINT:
    LOG_DEBUG("\tCreating IDT gate: x87 Floating-Point Exception (%d)", INTERRUPT_X87_FLOATING_POINT);
    break;
  case INTERRUPT_ALIGNMENT_CHECK:
    LOG_DEBUG("\tCreating IDT gate: Alignment Check Exception (%d)", INTERRUPT_ALIGNMENT_CHECK);
    break;
  case INTERRUPT_MACHINE_CHECK:
    LOG_DEBUG("\tCreating IDT gate: Machine Check Exception (%d)", INTERRUPT_MACHINE_CHECK);
    break;
  case INTERRUPT_SIMD_FLOATING_POINT:
    LOG_DEBUG("\tCreating IDT gate: SIMD Floating-Point Exception (%d)", INTERRUPT_SIMD_FLOATING_POINT);
    break;
  case INTERRUPT_VIRTUALIZATION:
    LOG_DEBUG("\tCreating IDT gate: Virtualization Exception (%d)", INTERRUPT_VIRTUALIZATION);
    break;
  case INTERRUPT_CONTROL_PROTECTION:
    LOG_DEBUG("\tCreating IDT gate: Control Protection Exception (%d)", INTERRUPT_CONTROL_PROTECTION);
    break;
  case INTERRUPT_RESERVED_22:
    LOG_DEBUG("\tCreating IDT gate: Reserved Exception 22 (%d)", INTERRUPT_RESERVED_22);
    break;
  case INTERRUPT_RESERVED_23:
    LOG_DEBUG("\tCreating IDT gate: Reserved Exception 23 (%d)", INTERRUPT_RESERVED_23);
    break;
  case INTERRUPT_RESERVED_24:
    LOG_DEBUG("\tCreating IDT gate: Reserved Exception 24 (%d)", INTERRUPT_RESERVED_24);
    break;
  case INTERRUPT_RESERVED_25:
    LOG_DEBUG("\tCreating IDT gate: Reserved Exception 25 (%d)", INTERRUPT_RESERVED_25);
    break;
  case INTERRUPT_RESERVED_26:
    LOG_DEBUG("\tCreating IDT gate: Reserved Exception 26 (%d)", INTERRUPT_RESERVED_26);
    break;
  case INTERRUPT_RESERVED_27:
    LOG_DEBUG("\tCreating IDT gate: Reserved Exception 27 (%d)", INTERRUPT_RESERVED_27);
    break;
  case INTERRUPT_RESERVED_28:
    LOG_DEBUG("\tCreating IDT gate: Reserved Exception 28 (%d)", INTERRUPT_RESERVED_28);
    break;
  case INTERRUPT_RESERVED_29:
    LOG_DEBUG("\tCreating IDT gate: Reserved Exception 29 (%d)", INTERRUPT_RESERVED_29);
    break;
  case INTERRUPT_RESERVED_30:
    LOG_DEBUG("\tCreating IDT gate: Reserved Exception 30 (%d)", INTERRUPT_RESERVED_30);
    break;
  case INTERRUPT_RESERVED_31:
    LOG_DEBUG("\tCreating IDT gate: Reserved Exception 31 (%d)", INTERRUPT_RESERVED_31);
    break;

  case IRQ_PIT:
    LOG_DEBUG("\tCreating IDT gate: IRQ0 - PIT (Programmable Interval Timer) (%d)", IRQ_PIT);
    break;
  case IRQ_KEYBOARD:
    LOG_DEBUG("\tCreating IDT gate: IRQ1 - Keyboard Controller (%d)", IRQ_KEYBOARD);
    break;
  case IRQ_CASCADE:
    LOG_DEBUG("\tCreating IDT gate: IRQ2 - Cascade (used internally by the two PICs) (%d)", IRQ_CASCADE);
    break;
  case IRQ_COM2:
    LOG_DEBUG("\tCreating IDT gate: IRQ3 - COM2 (if enabled) (%d)", IRQ_COM2);
    break;
  case IRQ_COM1:
    LOG_DEBUG("\tCreating IDT gate: IRQ4 - COM1 (if enabled) (%d)", IRQ_COM1);
    break;
  case IRQ_LPT2:
    LOG_DEBUG("\tCreating IDT gate: IRQ5 - LPT2 (if enabled) (%d)", IRQ_LPT2);
    break;
  case IRQ_FLOPPY:
    LOG_DEBUG("\tCreating IDT gate: IRQ6 - Floppy Disk Controller (%d)", IRQ_FLOPPY);
    break;
  case IRQ_LPT1:
    LOG_DEBUG("\tCreating IDT gate: IRQ7 - LPT1 / Unreliable 'spurious' interrupt (%d)", IRQ_LPT1);
    break;
  case IRQ_CMOS:
    LOG_DEBUG("\tCreating IDT gate: IRQ8 - CMOS real-time clock (%d)", IRQ_CMOS);
    break;
  case IRQ_FREE_41:
    LOG_DEBUG("\tCreating IDT gate: IRQ9 - Free for peripherals / legacy SCSI / NIC (%d)", IRQ_FREE_41);
    break;
  case IRQ_FREE_42:
    LOG_DEBUG("\tCreating IDT gate: IRQ10 - Free for peripherals / SCSI / NIC (%d)", IRQ_FREE_42);
    break;
  case IRQ_FREE_43:
    LOG_DEBUG("\tCreating IDT gate: IRQ11 - Free for peripherals / SCSI / NIC (%d)", IRQ_FREE_43);
    break;
  case IRQ_PS2_MOUSE:
    LOG_DEBUG("\tCreating IDT gate: IRQ12 - PS2 Mouse Controller (%d)", IRQ_PS2_MOUSE);
    break;
  case IRQ_FPU:
    LOG_DEBUG("\tCreating IDT gate: IRQ13 - FPU / Coprocessor / Inter-processor (%d)", IRQ_FPU);
    break;
  case IRQ_PRIMARY_ATA:
    LOG_DEBUG("\tCreating IDT gate: IRQ14 - Primary ATA Hard Disk Controller (%d)", IRQ_PRIMARY_ATA);
    break;
  case IRQ_SECONDARY_ATA:
    LOG_DEBUG("\tCreating IDT gate: IRQ15 - Secondary ATA Hard Disk Controller (%d)", IRQ_SECONDARY_ATA);
    break;
  case SYSCALL_INT_IDX:
    LOG_DEBUG("\tCreating IDT gate: System Call Interrupt (%d)", SYSCALL_INT_IDX);
    break;

  default:
    LOG_DEBUG("\tCreating IDT gate: Unknown Interrupt %d", n);
    break;
  }

  idt[n].handler_low = handler & 0x0000FFFF;
  idt[n].handler_high = (handler >> 16) & 0x0000FFFF;
  idt[n].segsel = SEGSEL_KERNEL_CS;
  idt[n].zero = 0;
  idt[n].config = (0x01 << 7) | ((pl & 0x03) << 5) | (0x01 << 3) | (0x01 << 2) | (0x01 << 1) | type;
}

void idt_init(void) {
  idt_ptr_t idt_ptr;
  idt_ptr.limit = IDT_NUM_ENTRIES * sizeof(idt_gate_t) - 1;
  idt_ptr.base = (uint32_t)&idt;

  /* Protected mode exceptions */
  LOG_INFO("Setting up protected mode exceptions");
  CREATE_IDT_GATE(0);
  CREATE_IDT_GATE(1);
  CREATE_IDT_GATE(2);
  CREATE_IDT_GATE(3);
  CREATE_IDT_GATE(4);
  CREATE_IDT_GATE(5);
  CREATE_IDT_GATE(6);
  CREATE_IDT_GATE(7);
  CREATE_IDT_GATE(8);
  CREATE_IDT_GATE(9);
  CREATE_IDT_GATE(10);
  CREATE_IDT_GATE(11);
  CREATE_IDT_GATE(12);
  CREATE_IDT_GATE(13);
  CREATE_IDT_GATE(14);
  CREATE_IDT_GATE(15);
  CREATE_IDT_GATE(16);
  CREATE_IDT_GATE(17);
  CREATE_IDT_GATE(18);
  CREATE_IDT_GATE(19);

  /* interrupts */
  LOG_INFO("Setting up interrupts");
  CREATE_IDT_GATE(20);
  CREATE_IDT_GATE(21);
  CREATE_IDT_GATE(22);
  CREATE_IDT_GATE(23);
  CREATE_IDT_GATE(24);
  CREATE_IDT_GATE(25);
  CREATE_IDT_GATE(26);
  CREATE_IDT_GATE(27);
  CREATE_IDT_GATE(28);
  CREATE_IDT_GATE(29);
  CREATE_IDT_GATE(30);
  CREATE_IDT_GATE(31);

  /* IRQs */
  LOG_INFO("Setting up IRQs");
  CREATE_IDT_GATE(32);
  CREATE_IDT_GATE(33);
  CREATE_IDT_GATE(34);
  CREATE_IDT_GATE(35);
  CREATE_IDT_GATE(36);
  CREATE_IDT_GATE(37);
  CREATE_IDT_GATE(38);
  CREATE_IDT_GATE(39);
  CREATE_IDT_GATE(40);
  CREATE_IDT_GATE(41);
  CREATE_IDT_GATE(42);
  CREATE_IDT_GATE(43);
  CREATE_IDT_GATE(44);
  CREATE_IDT_GATE(45);
  CREATE_IDT_GATE(46);
  CREATE_IDT_GATE(47);

  /* System call */
  create_idt_gate(SYSCALL_INT_IDX, (uint32_t)&interrupt_handler_128, IDT_TRAP_GATE_TYPE, PL3);

  load_idt((uint32_t)&idt_ptr);
  LOG_INFO("IDT initialized");
  LOG_LINE();
}
