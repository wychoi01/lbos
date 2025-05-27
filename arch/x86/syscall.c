#include "arch/x86/syscall.h"
#include "arch/x86/idt.h"
#include "lib/log.h"
#include "lib/string.h"
#include "mem/process.h"
#include "mem/paging.h"
#include "mem/page_frame_allocator.h"
#include <stdarg.h>

#define MAX_SYSCALLS 32

#define SYS_PRINTF 1
#define SYS_FORK 2
#define SYS_EXIT 3
#define SYS_WAIT 4

static syscall_handler_t syscall_handlers[MAX_SYSCALLS] = {0};

void syscall_init(void) {
    register_interrupt_handler(SYSCALL_INT_IDX, syscall_interrupt_handler);

    register_syscall(SYS_PRINTF, (syscall_handler_t)sys_printf);
    register_syscall(SYS_FORK, (syscall_handler_t)sys_fork);
    register_syscall(SYS_EXIT, (syscall_handler_t)sys_exit);
    register_syscall(SYS_WAIT, (syscall_handler_t)sys_wait);

    LOG_INFO("Syscall interface initialized");
}

void register_syscall(uint32_t num, syscall_handler_t handler) {
    if (num < MAX_SYSCALLS) {
        syscall_handlers[num] = handler;
    } else {
        LOG_ERROR("Cannot register syscall %d: out of range", num);
    }
}

void syscall_interrupt_handler(cpu_state_t state, idt_info_t info, stack_state_t exec) {
    (void)info;
    (void)exec;

    uint32_t syscall_num = state.eax;
    uint32_t arg1 = state.ebx;
    uint32_t arg2 = state.ecx;
    uint32_t arg3 = state.edx;
    uint32_t arg4 = state.esi;
    uint32_t arg5 = state.edi;

    LOG_DEBUG("Syscall %d received from user mode (args: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x)",
              syscall_num, arg1, arg2, arg3, arg4, arg5);

    if (syscall_num >= MAX_SYSCALLS || syscall_handlers[syscall_num] == NULL) {
        LOG_ERROR("Invalid syscall: %d", syscall_num);
        state.eax = (uint32_t)-1;
        return;
    }

    uint32_t result = syscall_handlers[syscall_num](arg1, arg2, arg3, arg4, arg5);
    LOG_DEBUG("Syscall %d returning result: 0x%x\n", syscall_num, result);

    state.eax = result;
}

uint32_t sys_printf(uint32_t fmt_ptr, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
    (void)arg1;
    (void)arg2;
    (void)arg3;
    (void)arg4;

    if (!fmt_ptr) {
        LOG_ERROR("Printf called with NULL format pointer");
        return (uint32_t)-1;
    }

    const char* fmt = (const char*)fmt_ptr;

    LOG_INFO("%s", fmt);
    return 1;
}

uint32_t sys_fork(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5) {
    (void)arg1;
    (void)arg2;
    (void)arg3;
    (void)arg4;
    (void)arg5;

    if (!current_process) {
        LOG_ERROR("Fork called with no current process");
        return (uint32_t)-1;
    }

    LOG_INFO("Fork called by PID %d", current_process->pid);

    uint32_t new_pid;
    process_t* child = allocate_pcb_and_pid(&new_pid);
    if (!child) {
        LOG_ERROR("Failed to allocate PCB for fork");
        return (uint32_t)-1;
    }

    child->parent_pid = current_process->pid;

    uint32_t* child_page_dir = create_page_directory();
    if (!child_page_dir) {
        LOG_ERROR("Failed to create page directory for fork");
        child->state = PROCESS_STATE_FREE;
        return (uint32_t)-1;
    }
    child->context.cr3 = virt_to_phys((uint32_t)child_page_dir);

    child->context.stack = current_process->context.stack;
    child->context.reg = current_process->context.reg;

    uint32_t* parent_page_dir = (uint32_t*)phys_to_virt(current_process->context.cr3);

    for (uint32_t pde_idx = 0; pde_idx < (KERNEL_VIRTUAL_START / 0x400000); pde_idx++) {
        if (parent_page_dir[pde_idx] & PAGE_PRESENT) {
            uint32_t* parent_page_table = (uint32_t*)phys_to_virt(parent_page_dir[pde_idx] & ~0xFFF);

            for (uint32_t pte_idx = 0; pte_idx < 1024; pte_idx++) {
                if (parent_page_table[pte_idx] & PAGE_PRESENT) {
                    uint32_t virt_addr = (pde_idx << 22) | (pte_idx << 12);
                    uint32_t phys_addr = parent_page_table[pte_idx] & ~0xFFF;

                    uint32_t child_frame = alloc_frame();
                    if (child_frame == 0) {
                        LOG_ERROR("Failed to allocate frame for child process");
                        child->state = PROCESS_STATE_FREE;
                        return (uint32_t)-1;
                    }

                    memcpy((void*)phys_to_virt(child_frame),
                           (void*)phys_to_virt(phys_addr),
                           FRAME_SIZE);

                    uint32_t flags = parent_page_table[pte_idx] & 0xFFF;
                    map_page((uint32_t*)child->context.cr3, virt_addr, child_frame, flags);
                }
            }
        }
    }

    uintptr_t child_stack_top = (uintptr_t)child->kstack + PROCESS_KERNEL_STACK_SIZE;
    uint32_t* child_kstack_ptr = (uint32_t*)child_stack_top;

    *--child_kstack_ptr = child->context.stack.ss;
    *--child_kstack_ptr = child->context.stack.esp;
    *--child_kstack_ptr = child->context.stack.eflags;
    *--child_kstack_ptr = child->context.stack.cs;
    *--child_kstack_ptr = child->context.stack.eip;

    child->context.reg.eax = 0;
    child->context.reg.esp = (uint32_t)child_kstack_ptr;

    child->state = PROCESS_STATE_READY;
    child->next_in_ready_queue = ready_queue_head;
    ready_queue_head = child;

    LOG_INFO("Fork successful: parent PID %d -> child PID %d", current_process->pid, child->pid);

    return child->pid;
}

uint32_t sys_exit(uint32_t status, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5) {
    (void)arg2;
    (void)arg3;
    (void)arg4;
    (void)arg5;

    if (!current_process) {
        LOG_ERROR("Exit called with no current process");
        return (uint32_t)-1;
    }

    LOG_INFO("Process %d exiting with status %d", current_process->pid, status);

    current_process->context.reg.eax = status;
    current_process->state = PROCESS_STATE_TERMINATED;

    schedule();

    return 0;
}

uint32_t sys_wait(uint32_t status_ptr, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5) {
    (void)arg2;
    (void)arg3;
    (void)arg4;
    (void)arg5;

    if (!current_process) {
        LOG_ERROR("Wait called with no current process");
        return (uint32_t)-1;
    }

    LOG_INFO("Process %d waiting for child", current_process->pid);

    process_t* child = get_zombie_process_for_parent(current_process->pid);

    if (!child) {
        LOG_WARN("No terminated children found");
        return -1;
    }

    int exit_status = child->context.reg.eax;
    if (status_ptr) {
        int* user_status = (int*)status_ptr;
        *user_status = exit_status;
    }

    child->state = PROCESS_STATE_FREE;
    child->pid = 0;
    child->parent_pid = 0;

    LOG_INFO("Process %d reaped child %d with status %d",
             current_process->pid, child->pid, exit_status);

    return child->pid;
}
