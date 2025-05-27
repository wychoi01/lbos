#include "arch/x86/interrupt.h"
#include "arch/x86/idt.h"
#include "arch/x86/io.h"
#include "arch/x86/pic.h"
#include "arch/x86/pit.h"
#include "lib/log.h"
#include "mem/process.h"
#include <stddef.h>

static interrupt_handler_t interrupt_handlers[IDT_NUM_ENTRIES];

uint32_t register_interrupt_handler(uint32_t interrupt, interrupt_handler_t handler) {
  if (interrupt >= IDT_NUM_ENTRIES || interrupt_handlers[interrupt] != NULL) {
    return 1;
  }

  interrupt_handlers[interrupt] = handler;

  return 0;
}

void general_protection_fault_handler(cpu_state_t state, idt_info_t info, stack_state_t exec) {
    LOG_ERROR("General Protection Fault!");
    LOG_ERROR("Error code: 0x%x", info.error_code);
    LOG_ERROR("EIP: 0x%x, CS: 0x%x, EFLAGS: 0x%x", exec.eip, exec.cs, exec.eflags);

    if ((exec.cs & 0x3) != 0 || (info.error_code & 0x1)) {
        LOG_ERROR("ESP: 0x%x, SS: 0x%x", exec.esp, exec.ss);
    }

    LOG_ERROR("Registers:");
    LOG_ERROR("  EAX: 0x%x, EBX: 0x%x, ECX: 0x%x, EDX: 0x%x",
              state.eax, state.ebx, state.ecx, state.edx);
    LOG_ERROR("  ESI: 0x%x, EDI: 0x%x, EBP: 0x%x, ESP: 0x%x",
              state.esi, state.edi, state.ebp, state.esp);

    if (current_process) {
        LOG_ERROR("Current process: PID %d", current_process->pid);
    }

    if (info.error_code != 0) {
        LOG_ERROR("Error code breakdown:");
        LOG_ERROR("  External: %s", (info.error_code & 0x1) ? "Yes" : "No");
        LOG_ERROR("  IDT/GDT/LDT: %s", (info.error_code & 0x2) ? "IDT" : "GDT/LDT");
        LOG_ERROR("  Selector index: %d", (info.error_code >> 3) & 0x1FFF);
    }

    LOG_ERROR("System halted due to GPF");
    while (1) {
        __asm__("hlt");
    }
}

void interrupt_handler(cpu_state_t state, idt_info_t info, stack_state_t exec) {
  if (info.idt_index == SYSCALL_INT_IDX) {
    if (interrupt_handlers[info.idt_index] != NULL) {
      interrupt_handlers[info.idt_index](state, info, exec);
    } else {
      LOG_ERROR("System call handler not registered! (interrupt 0x%x, eip: 0x%x)",
                info.idt_index, exec.eip);
    }
  } else if (interrupt_handlers[info.idt_index] != NULL) {
    interrupt_handlers[info.idt_index](state, info, exec);
  } else {
    LOG_ERROR("Unhandled interrupt: %x, eip: %x, cs: %x, eflags: %x",
              info.idt_index, exec.eip, exec.cs, exec.eflags);
  }
}

void interrupt_init(void) {
  pic_init();
  idt_init();
  pit_init();

  register_interrupt_handler(INTERRUPT_GENERAL_PROTECTION_FAULT, general_protection_fault_handler);
}
