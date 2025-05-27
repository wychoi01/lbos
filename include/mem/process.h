#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

#define MAX_PROCESSES 64
#define PROCESS_KERNEL_STACK_SIZE 4096
#define KERNEL_CS_SELECTOR 0x08
#define KERNEL_DS_SELECTOR 0x10
#define KERNEL_EFLAGS 0x202
#define USER_CS_SELECTOR 0x1B
#define USER_DS_SELECTOR 0x23
#define USER_EFLAGS 0x202
#define USER_STACK_TOP 0xBFFFF000
#define USER_CODE_START 0x08048000
#define USER_HEAP_START 0x08100000

typedef enum {
  PROCESS_STATE_FREE,
  PROCESS_STATE_READY,
  PROCESS_STATE_RUNNING,
  PROCESS_STATE_BLOCKED,
  PROCESS_STATE_TERMINATED
} process_state_t;

struct cpu_state {
  uint32_t edi;
  uint32_t esi;
  uint32_t ebp;
  uint32_t edx;
  uint32_t ecx;
  uint32_t ebx;
  uint32_t eax;
  uint32_t esp;
} __attribute__((packed));
typedef struct cpu_state cpu_state_t;

struct stack_state {
  uint32_t eip;
  uint32_t cs;
  uint32_t eflags;
  uint32_t esp;
  uint32_t ss;
} __attribute__((packed));
typedef struct stack_state stack_state_t;

typedef struct {
  cpu_state_t reg;
  stack_state_t stack;
  uint32_t cr3;
} process_context_t;

typedef struct process {
  uint32_t pid;
  process_state_t state;
  process_context_t context;
  uint8_t kstack[PROCESS_KERNEL_STACK_SIZE] __attribute__((aligned(16)));
  struct process* next_in_ready_queue;
  uint32_t parent_pid;
  int exit_status;
} process_t;

void init_process_manager(void);
process_t* allocate_pcb_and_pid(uint32_t* new_pid);
process_t* create_process(void* module_data, uint32_t module_size);
process_t* create_kernel_process(void (*entry_point)(void));
process_t* get_zombie_process_for_parent(uint32_t parent_pid);
void switch_to_process(process_t* next);
void schedule(void);

extern process_t* current_process;
extern process_t* ready_queue_head;

#endif /* PROCESS_H */
