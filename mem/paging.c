#include "mem/paging.h"
#include "lib/log.h"
#include "lib/string.h"
#include "mem/page_frame_allocator.h"

static uint32_t kernel_page_directory[PAGE_DIRECTORY_SIZE] __attribute__((aligned(4096)));
static uint32_t kernel_page_table[PAGE_TABLE_SIZE] __attribute__((aligned(4096)));

void init_paging(void) {
  memset(kernel_page_directory, 0, sizeof(kernel_page_directory));
  memset(kernel_page_table, 0, sizeof(kernel_page_table));

  kernel_page_directory[0] = ((uint32_t)kernel_page_table) | PAGE_PRESENT | PAGE_RW | PAGE_WRITETHROUGH;
  kernel_page_directory[KERNEL_PDT_IDX] = ((uint32_t)kernel_page_table) | PAGE_PRESENT | PAGE_RW | PAGE_WRITETHROUGH;

  for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
    kernel_page_table[i] = (i * FRAME_SIZE) | PAGE_PRESENT | PAGE_RW | PAGE_WRITETHROUGH;
  }
}

void enable_paging(void) {
  asm volatile("movl %0, %%cr3" ::"r"(kernel_page_directory));

  uint32_t cr4;
  asm volatile("movl %%cr4, %0" : "=r"(cr4));
  cr4 |= 0x00000010;
  asm volatile("movl %0, %%cr4" ::"r"(cr4));

  uint32_t cr0;
  asm volatile("movl %%cr0, %0" : "=r"(cr0));
  cr0 |= 0x80000000;
  asm volatile("movl %0, %%cr0" ::"r"(cr0));
}

void setup_higher_half(void) {
  kernel_page_directory[0] = 0;
  asm volatile("invlpg (%0)" ::"r"(0));
}

uint32_t* create_page_directory(void) {
  uint32_t page_dir_phys = alloc_frame();
  if (page_dir_phys == 0) {
    LOG_ERROR("Failed to allocate frame for page directory");
    return 0;
  }

  uint32_t* new_page_directory = (uint32_t*)phys_to_virt(page_dir_phys);

  memset(new_page_directory, 0, PAGE_DIRECTORY_SIZE * sizeof(uint32_t));

  for (int i = KERNEL_PDT_IDX; i < PAGE_DIRECTORY_SIZE; i++) {
    new_page_directory[i] = kernel_page_directory[i];
  }

  return new_page_directory;
}

void map_page(uint32_t* page_directory, uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags) {
  uint32_t pd_index = virtual_addr >> 22;
  uint32_t pt_index = (virtual_addr >> 12) & 0x3FF;

  uint32_t* pd_virt = (uint32_t*)phys_to_virt((uint32_t)page_directory);

  if (!(pd_virt[pd_index] & PAGE_PRESENT)) {
    uint32_t pt_phys = alloc_frame();
    if (pt_phys == 0) {
      LOG_ERROR("Failed to allocate frame for page table");
      return;
    }

    uint32_t* pt_virt = (uint32_t*)phys_to_virt(pt_phys);
    memset(pt_virt, 0, PAGE_TABLE_SIZE * sizeof(uint32_t));

    pd_virt[pd_index] = pt_phys | PAGE_PRESENT | PAGE_RW | PAGE_USER;
  }

  uint32_t pt_phys = pd_virt[pd_index] & ~0xFFF;
  uint32_t* pt_virt = (uint32_t*)phys_to_virt(pt_phys);

  pt_virt[pt_index] = physical_addr | flags;

  asm volatile("invlpg (%0)" :: "r"(virtual_addr) : "memory");
}

void unmap_page(uint32_t* page_directory, uint32_t virtual_addr) {
  uint32_t pd_index = virtual_addr >> 22;
  uint32_t pt_index = (virtual_addr >> 12) & 0x3FF;

  uint32_t* pd_virt = (uint32_t*)phys_to_virt((uint32_t)page_directory);

  if (!(pd_virt[pd_index] & PAGE_PRESENT)) {
    return;
  }

  uint32_t pt_phys = pd_virt[pd_index] & ~0xFFF;
  uint32_t* pt_virt = (uint32_t*)phys_to_virt(pt_phys);

  pt_virt[pt_index] = 0;

  asm volatile("invlpg (%0)" :: "r"(virtual_addr) : "memory");
}

uint32_t get_physical_address(uint32_t* page_directory, uint32_t virtual_addr) {
  uint32_t pd_index = virtual_addr >> 22;
  uint32_t pt_index = (virtual_addr >> 12) & 0x3FF;
  uint32_t offset = virtual_addr & 0xFFF;

  uint32_t* pd_virt = (uint32_t*)phys_to_virt((uint32_t)page_directory);

  if (!(pd_virt[pd_index] & PAGE_PRESENT)) {
    return 0;
  }

  uint32_t pt_phys = pd_virt[pd_index] & ~0xFFF;
  uint32_t* pt_virt = (uint32_t*)phys_to_virt(pt_phys);

  if (!(pt_virt[pt_index] & PAGE_PRESENT)) {
    return 0;
  }

  uint32_t page_phys = pt_virt[pt_index] & ~0xFFF;
  return page_phys | offset;
}
