#ifndef SYSCALL_H
#define SYSCALL_H

#include "arch/x86/interrupt.h"
#include <stdint.h>

#define SYS_PRINTF 1
#define SYS_FORK 2
#define SYS_EXIT 3
#define SYS_WAIT 4

typedef uint32_t (*syscall_handler_t)(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5);

void syscall_init(void);
void register_syscall(uint32_t num, syscall_handler_t handler);
void syscall_interrupt_handler(cpu_state_t state, idt_info_t info, stack_state_t exec);

uint32_t sys_printf(uint32_t fmt_ptr, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);
uint32_t sys_fork(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5);
uint32_t sys_exit(uint32_t status, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5);
uint32_t sys_wait(uint32_t status_ptr, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5);

#endif /* SYSCALL_H */
