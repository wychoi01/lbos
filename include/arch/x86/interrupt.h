#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "mem/process.h"
#include <stdint.h>

struct idt_info {
  uint32_t idt_index;
  uint32_t error_code;
} __attribute__((packed));
typedef struct idt_info idt_info_t;

typedef void (*interrupt_handler_t)(cpu_state_t state, idt_info_t info, stack_state_t exec);
uint32_t register_interrupt_handler(uint32_t interrupt, interrupt_handler_t handler);

void interrupt_init(void);
void interrupt_sti(void);
void interrupt_cli(void);

#endif /* INTERRUPT_H */
