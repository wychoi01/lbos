#include "mem/page_frame_allocator.h"
#include "arch/x86/idt.h"
#include "arch/x86/interrupt.h"
#include "lib/log.h"
#include "lib/string.h"
#include "mem/paging.h"
#include "mem/process.h"
#include <stdbool.h>

static uint32_t* frame_bitmap = NULL;
static uint32_t bitmap_size = 0;
static uint32_t total_frames = 0;

static uint32_t kernel_physical_start = 0;
static uint32_t kernel_physical_end = 0;
static uint32_t kernel_virtual_start = 0;
static uint32_t kernel_virtual_end = 0;

extern void page_fault_handler(cpu_state_t state, idt_info_t info, stack_state_t exec);

static void set_bit(uint32_t frame_idx) {
  uint32_t byte_idx = frame_idx / BITS_PER_BYTE;
  uint32_t bit_idx = frame_idx % BITS_PER_BYTE;
  frame_bitmap[byte_idx] |= (1 << bit_idx);
}

static void clear_bit(uint32_t frame_idx) {
  uint32_t byte_idx = frame_idx / BITS_PER_BYTE;
  uint32_t bit_idx = frame_idx % BITS_PER_BYTE;
  frame_bitmap[byte_idx] &= ~(1 << bit_idx);
}

static bool test_bit(uint32_t frame_idx) {
  uint32_t byte_idx = frame_idx / BITS_PER_BYTE;
  uint32_t bit_idx = frame_idx % BITS_PER_BYTE;
  return (frame_bitmap[byte_idx] & (1 << bit_idx)) != 0;
}

uint32_t phys_to_virt(uint32_t phys_addr) { return phys_addr + (kernel_virtual_start - kernel_physical_start); }
uint32_t virt_to_phys(uint32_t virt_addr) { return virt_addr - (kernel_virtual_start - kernel_physical_start); }

void init_page_frame_allocator(uint32_t phys_start, uint32_t phys_end, uint32_t virt_start, uint32_t virt_end) {
  LOG_INFO("Initializing page frame allocator");

  kernel_physical_start = phys_start;
  kernel_physical_end = phys_end;
  kernel_virtual_start = virt_start;
  kernel_virtual_end = virt_end;

  uint32_t kernel_physical_size = kernel_physical_end - kernel_physical_start;
  uint32_t kernel_virtual_size = kernel_virtual_end - kernel_virtual_start;
  uint32_t total_physical_memory = (1024 * 1024 * 128);

  total_frames = total_physical_memory / FRAME_SIZE;
  uint32_t reserved_kernel_frames = (kernel_physical_end - kernel_physical_start) / FRAME_SIZE;
  uint32_t reserved_low_memory_frames = kernel_physical_start / FRAME_SIZE;
  uint32_t available_frames = total_frames - reserved_kernel_frames - reserved_low_memory_frames;

  LOG_DEBUG("Memory Structure Information:");
  LOG_DEBUG("\tPhysical Memory:");
  LOG_DEBUG("\t\tTotal Physical Memory: %d MB", total_physical_memory / (1024 * 1024));
  LOG_DEBUG("\t\tKernel Physical Start: 0x%x", kernel_physical_start);
  LOG_DEBUG("\t\tKernel Physical End: 0x%x", kernel_physical_end);
  LOG_DEBUG("\t\tKernel Physical Size: %d KB", kernel_physical_size / 1024);
  LOG_DEBUG("\tVirtual Memory:");
  LOG_DEBUG("\t\tKernel Virtual Start: 0x%x", kernel_virtual_start);
  LOG_DEBUG("\t\tKernel Virtual End: 0x%x", kernel_virtual_end);
  LOG_DEBUG("\t\tKernel Virtual Size: %d KB", kernel_virtual_size / 1024);
  LOG_DEBUG("\tPage Frame Information:");
  LOG_DEBUG("\t\tFrame Size: %d", FRAME_SIZE);
  LOG_DEBUG("\t\tTotal Frames: %d", total_frames);
  LOG_DEBUG("\t\tAvailable Frames: %d", available_frames);
  LOG_DEBUG("\t\tFrames Used by Kernel: %d", reserved_kernel_frames);
  LOG_DEBUG("\t\tReserved Low Memory Frames: %d", reserved_low_memory_frames);

  bitmap_size = ((total_frames + BITS_PER_BYTE - 1) / BITS_PER_BYTE + FRAME_SIZE - 1) & ~(FRAME_SIZE - 1);

  uint32_t bitmap_phys = (kernel_physical_end + FRAME_SIZE - 1) & ~(FRAME_SIZE - 1);
  frame_bitmap = (uint32_t*)phys_to_virt(bitmap_phys);
  LOG_DEBUG("\tFrame bitmap: 0x%x", frame_bitmap);
  LOG_DEBUG("\tBitmap size: %d bytes", bitmap_size);

  memset(frame_bitmap, 0, bitmap_size);

  uint32_t kernel_start_frame = kernel_physical_start / FRAME_SIZE;
  uint32_t bitmap_end_phys = bitmap_phys + bitmap_size;
  uint32_t kernel_end_frame = bitmap_end_phys / FRAME_SIZE;

  LOG_DEBUG("\tKernel start frame: 0x%x", kernel_start_frame);
  LOG_DEBUG("\tKernel end frame: 0x%x", kernel_end_frame);

  for (uint32_t i = 0; i < kernel_start_frame; i++) {
    set_bit(i);
  }

  for (uint32_t i = kernel_start_frame; i < kernel_end_frame; i++) {
    set_bit(i);
  }

  register_interrupt_handler(INTERRUPT_PAGE_FAULT, page_fault_handler);

  LOG_INFO("Page frame allocator initialized");
  LOG_LINE();
}

uint32_t alloc_frame(void) {
  for (uint32_t i = 0; i < total_frames; i++) {
    if (!test_bit(i)) {
      set_bit(i);
      return i * FRAME_SIZE;
    }
  }
  return 0;
}

void free_frame(uint32_t frame_addr) {
  uint32_t frame_idx = frame_addr / FRAME_SIZE;
  if (frame_idx < total_frames) {
    clear_bit(frame_idx);
  }
}

bool is_frame_allocated(uint32_t frame_addr) {
  uint32_t frame_idx = frame_addr / FRAME_SIZE;
  if (frame_idx < total_frames) {
    return test_bit(frame_idx);
  }
  return true;
}

void page_fault_handler(cpu_state_t state, idt_info_t info, stack_state_t exec) {
  (void)state;

  uint32_t faulting_address;
  asm volatile("mov %%cr2, %0" : "=r"(faulting_address));

  uint32_t error_code = info.error_code;
  bool present = error_code & 0x1;
  bool write = error_code & 0x2;
  bool user = error_code & 0x4;

  LOG_DEBUG("Page fault at virtual address: 0x%x, error_code: 0x%x (present=%d, write=%d, user=%d)",
            faulting_address, error_code, present, write, user);

  LOG_DEBUG("  Fault details:");
  LOG_DEBUG("    EIP: 0x%x", exec.eip);
  LOG_DEBUG("    CS: 0x%x", exec.cs);
  LOG_DEBUG("    Current process: PID %d", current_process ? current_process->pid : (uint32_t)-1);
  LOG_DEBUG("    Address region: %s",
            faulting_address >= USER_STACK_TOP - 0x100000 && faulting_address < USER_STACK_TOP ? "User stack" :
            faulting_address >= USER_CODE_START && faulting_address < USER_HEAP_START ? "User code/data" :
            faulting_address >= KERNEL_VIRTUAL_START ? "Kernel space" : "Unknown");

  if (!current_process) {
    LOG_ERROR("Page fault with no current process at address: 0x%x", faulting_address);
    while (1) __asm__("hlt");
    return;
  }

  if (faulting_address >= KERNEL_VIRTUAL_START) {
    LOG_ERROR("Page fault in kernel space at address: 0x%x, eip: 0x%x", faulting_address, exec.eip);
    while (1) __asm__("hlt");
    return;
  }

  if (!present) {
    uint32_t page_addr = faulting_address & ~0xFFF;

    uint32_t frame_phys = alloc_frame();
    if (frame_phys == 0) {
      LOG_ERROR("Failed to allocate frame for page fault at address: 0x%x", faulting_address);
      while (1) __asm__("hlt");
      return;
    }

    uint32_t flags = PAGE_PRESENT | PAGE_USER;
    if (write || page_addr < 0xC0000000) {
      flags |= PAGE_RW;
    }

    uint32_t* page_dir = (uint32_t*)current_process->context.cr3;
    map_page(page_dir, page_addr, frame_phys, flags);

    uint32_t* page_virt = (uint32_t*)phys_to_virt(frame_phys);
    memset(page_virt, 0, FRAME_SIZE);

    LOG_DEBUG("Successfully mapped virtual address 0x%x to physical frame 0x%x for PID %d",
              page_addr, frame_phys, current_process->pid);
    LOG_DEBUG("  Page flags: 0x%x (Present=%d, RW=%d, User=%d)",
              flags, (flags & PAGE_PRESENT) ? 1 : 0,
              (flags & PAGE_RW) ? 1 : 0, (flags & PAGE_USER) ? 1 : 0);

    return;
  }

  LOG_ERROR("Page fault (protection violation) at virtual address: 0x%x, eip: 0x%x, error_code: 0x%x",
            faulting_address, exec.eip, error_code);

  while (1) __asm__("hlt");

  return;
}
