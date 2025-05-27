#include "mem/process.h"
#include "arch/x86/tss.h"
#include "lib/log.h"
#include "lib/string.h"
#include "mem/page_frame_allocator.h"
#include "mem/paging.h"

#include <stddef.h>
#include <stdint.h>

static uint32_t next_pid = 1;
static process_t process_table[MAX_PROCESSES];

process_t* current_process = NULL;
process_t* ready_queue_head = NULL;

process_t* allocate_pcb_and_pid(uint32_t* new_pid) {
  for (int i = 0; i < MAX_PROCESSES; ++i) {
    if (process_table[i].state == PROCESS_STATE_FREE) {
      *new_pid = next_pid++;
      process_table[i].pid = *new_pid;
      memset(&process_table[i].context, 0, sizeof(process_context_t));
      memset(process_table[i].kstack, 0xCD, PROCESS_KERNEL_STACK_SIZE);
      return &process_table[i];
    }
  }
  LOG_ERROR("No free PCBs available!");
  return NULL;
}

process_t* get_zombie_process_for_parent(uint32_t parent_pid) {
  for (int i = 0; i < MAX_PROCESSES; i++) {
    if (process_table[i].state == PROCESS_STATE_TERMINATED &&
        process_table[i].parent_pid == parent_pid) {
      return &process_table[i];
    }
  }
  return NULL;
}

void kernel_idle(void) {
  while (1)
    __asm__("hlt");
}

void init_process_manager(void) {
  LOG_INFO("Initializing process manager...");
  for (int i = 0; i < MAX_PROCESSES; ++i) {
    process_table[i].state = PROCESS_STATE_FREE;
    process_table[i].pid = 0;
    process_table[i].next_in_ready_queue = NULL;
  }
  current_process = NULL;
  ready_queue_head = NULL;
  next_pid = 1;
  LOG_INFO("Process manager initialized. Process table has %d slots.", MAX_PROCESSES);
  LOG_LINE();
}

process_t* create_process(void* module_data, uint32_t module_size) {
  uint32_t new_pid_val;
  process_t* new_proc = allocate_pcb_and_pid(&new_pid_val);

  if (new_proc == NULL) {
    return NULL;
  }

  LOG_INFO("Creating user process with PID: %d, module size: %d bytes", new_pid_val, module_size);

  uint32_t* page_dir_virtual = create_page_directory();
  if (page_dir_virtual == NULL) {
    LOG_ERROR("Failed to create page directory for PID %d", new_pid_val);
    new_proc->state = PROCESS_STATE_FREE;
    return NULL;
  }
  new_proc->context.cr3 = virt_to_phys((uint32_t)page_dir_virtual);
  LOG_DEBUG("PID %d: Page directory created. Virtual: 0x%x, Physical (CR3): 0x%x", new_pid_val,
            (uint32_t)page_dir_virtual, new_proc->context.cr3);

  uint32_t pages_needed = (module_size + FRAME_SIZE - 1) / FRAME_SIZE;
  uint32_t current_vaddr = USER_CODE_START;

  LOG_DEBUG("User process memory layout:");
  LOG_DEBUG("  Code section: 0x%x - 0x%x (%d bytes, %d pages)",
            USER_CODE_START, USER_CODE_START + (pages_needed * FRAME_SIZE) - 1,
            module_size, pages_needed);
  LOG_DEBUG("  Stack top: 0x%x", USER_STACK_TOP);
  LOG_DEBUG("  Stack bottom (will grow down): 0x%x", USER_STACK_TOP - FRAME_SIZE);

  for (uint32_t i = 0; i < pages_needed; i++) {
    uint32_t frame_phys = alloc_frame();
    if (frame_phys == 0) {
      LOG_ERROR("Failed to allocate frame for user code page %d", i);
      new_proc->state = PROCESS_STATE_FREE;
      return NULL;
    }

    map_page((uint32_t*)new_proc->context.cr3, current_vaddr, frame_phys,
             PAGE_PRESENT | PAGE_USER | PAGE_RW);

    LOG_DEBUG("  Mapped virtual 0x%x -> physical 0x%x (page %d)",
              current_vaddr, frame_phys, i);

    uint32_t offset = i * FRAME_SIZE;
    uint32_t copy_size = (offset + FRAME_SIZE > module_size) ?
                        (module_size - offset) : FRAME_SIZE;

    uint32_t* frame_virt = (uint32_t*)phys_to_virt(frame_phys);
    memcpy(frame_virt, (uint8_t*)module_data + offset, copy_size);

    if (copy_size < FRAME_SIZE) {
      memset((uint8_t*)frame_virt + copy_size, 0, FRAME_SIZE - copy_size);
    }

    current_vaddr += FRAME_SIZE;
  }

  LOG_DEBUG("Mapped %d pages for user code starting at 0x%x", pages_needed, USER_CODE_START);

  new_proc->context.stack.eip = USER_CODE_START;
  new_proc->context.stack.cs = USER_CS_SELECTOR;
  new_proc->context.stack.eflags = USER_EFLAGS;
  new_proc->context.stack.esp = USER_STACK_TOP;
  new_proc->context.stack.ss = USER_DS_SELECTOR;

  LOG_DEBUG("Process context setup:");
  LOG_DEBUG("  EIP: 0x%x (entry point)", new_proc->context.stack.eip);
  LOG_DEBUG("  CS: 0x%x (user code selector)", new_proc->context.stack.cs);
  LOG_DEBUG("  ESP: 0x%x (user stack pointer)", new_proc->context.stack.esp);
  LOG_DEBUG("  SS: 0x%x (user stack selector)", new_proc->context.stack.ss);
  LOG_DEBUG("  EFLAGS: 0x%x", new_proc->context.stack.eflags);
  LOG_DEBUG("  CR3: 0x%x (page directory physical)", new_proc->context.cr3);

  uintptr_t stack_top_addr = (uintptr_t)new_proc->kstack + PROCESS_KERNEL_STACK_SIZE;
  uint32_t* kstack_ptr = (uint32_t*)stack_top_addr;

  LOG_DEBUG("Kernel stack setup for iret:");
  LOG_DEBUG("  Kernel stack base: 0x%x", (uint32_t)new_proc->kstack);
  LOG_DEBUG("  Kernel stack top: 0x%x", stack_top_addr);

  *--kstack_ptr = new_proc->context.stack.ss;
  *--kstack_ptr = new_proc->context.stack.esp;
  *--kstack_ptr = new_proc->context.stack.eflags;
  *--kstack_ptr = new_proc->context.stack.cs;
  *--kstack_ptr = new_proc->context.stack.eip;

  LOG_DEBUG("  Stack contents (for iret):");
  LOG_DEBUG("    [0x%x] SS: 0x%x", (uint32_t)(kstack_ptr + 4), new_proc->context.stack.ss);
  LOG_DEBUG("    [0x%x] ESP: 0x%x", (uint32_t)(kstack_ptr + 3), new_proc->context.stack.esp);
  LOG_DEBUG("    [0x%x] EFLAGS: 0x%x", (uint32_t)(kstack_ptr + 2), new_proc->context.stack.eflags);
  LOG_DEBUG("    [0x%x] CS: 0x%x", (uint32_t)(kstack_ptr + 1), new_proc->context.stack.cs);
  LOG_DEBUG("    [0x%x] EIP: 0x%x", (uint32_t)kstack_ptr, new_proc->context.stack.eip);
  LOG_DEBUG("  Final kernel ESP: 0x%x", (uint32_t)kstack_ptr);

  new_proc->context.reg.esp = (uint32_t)kstack_ptr;

  new_proc->state = PROCESS_STATE_READY;
  new_proc->next_in_ready_queue = ready_queue_head;
  ready_queue_head = new_proc;

  return new_proc;
}

process_t* create_kernel_process(void (*entry_point)(void)) {
  uint32_t new_pid_val;
  process_t* new_proc = allocate_pcb_and_pid(&new_pid_val);

  if (new_proc == NULL) {
    return NULL;
  }

  LOG_INFO("Creating kernel process with PID: %d, entry point: 0x%x", new_pid_val, (uint32_t)entry_point);

  uint32_t* page_dir_virtual = create_page_directory();
  if (page_dir_virtual == NULL) {
    LOG_ERROR("Failed to create page directory for PID %d", new_pid_val);
    new_proc->state = PROCESS_STATE_FREE;
    return NULL;
  }
  new_proc->context.cr3 = virt_to_phys((uint32_t)page_dir_virtual);
  LOG_DEBUG("PID %d: Page directory created. Virtual: 0x%x, Physical (CR3): 0x%x", new_pid_val,
            (uint32_t)page_dir_virtual, new_proc->context.cr3);

  new_proc->context.stack.eip = (uint32_t)entry_point;
  new_proc->context.stack.cs = KERNEL_CS_SELECTOR;
  new_proc->context.stack.eflags = KERNEL_EFLAGS;
  new_proc->context.stack.esp = 0;
  new_proc->context.stack.ss = 0;

  uintptr_t stack_top_addr = (uintptr_t)new_proc->kstack + PROCESS_KERNEL_STACK_SIZE;
  uint32_t* kstack_ptr = (uint32_t*)stack_top_addr;

  *--kstack_ptr = (uint32_t)kernel_idle;
  *--kstack_ptr = new_proc->context.stack.eflags;
  *--kstack_ptr = new_proc->context.stack.cs;
  *--kstack_ptr = new_proc->context.stack.eip;

  new_proc->context.reg.esp = (uint32_t)kstack_ptr;

  new_proc->state = PROCESS_STATE_READY;
  new_proc->next_in_ready_queue = ready_queue_head;
  ready_queue_head = new_proc;

  return new_proc;
}

void schedule(void) {
  if (current_process) {
    LOG_DEBUG("schedule() called. Current PID: %d. Ready queue head PID: %d", current_process->pid,
              ready_queue_head ? ready_queue_head->pid : 0);
  } else {
    LOG_DEBUG("schedule() called. No current process. Ready queue head PID: %d",
              ready_queue_head ? ready_queue_head->pid : 0);
  }

  if (ready_queue_head != NULL) {
    LOG_DEBUG("Switching to process PID %d", ready_queue_head->pid);
    current_process = ready_queue_head;
    ready_queue_head = ready_queue_head->next_in_ready_queue;
    current_process->state = PROCESS_STATE_RUNNING;
    LOG_DEBUG("Switching to process PID %d", current_process->pid);

    uint32_t kernel_stack_top = (uint32_t)current_process->kstack + PROCESS_KERNEL_STACK_SIZE;
    tss_set_kernel_stack(kernel_stack_top);

    LOG_DEBUG("Context switch details:");
    LOG_DEBUG("  TSS kernel stack (ESP0): 0x%x", kernel_stack_top);
    LOG_DEBUG("  Process kernel ESP: 0x%x", current_process->context.reg.esp);
    LOG_DEBUG("  Target CR3: 0x%x", current_process->context.cr3);
    LOG_DEBUG("  Target EIP: 0x%x", current_process->context.stack.eip);
    LOG_DEBUG("  Target CS: 0x%x", current_process->context.stack.cs);
    LOG_DEBUG("  Target user ESP: 0x%x", current_process->context.stack.esp);
    LOG_DEBUG("  Target SS: 0x%x", current_process->context.stack.ss);

    switch_to_process(current_process);
  }
}
