#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

#define KERNEL_VIRTUAL_ADDRESS 0xC040000
#define KERNEL_VIRTUAL_START 0xC0000000
#define KERNEL_PDT_IDX (KERNEL_VIRTUAL_START >> 22)

#define PAGE_PRESENT 0x1
#define PAGE_RW 0x2
#define PAGE_USER 0x4
#define PAGE_WRITETHROUGH 0x8
#define PAGE_NOCACHE 0x10
#define PAGE_ACCESSED 0x20
#define PAGE_DIRTY 0x40
#define PAGE_SIZE_4MB 0x80
#define PAGE_GLOBAL 0x100

#define PAGE_DIRECTORY_SIZE 1024
#define PAGE_TABLE_SIZE 1024
#define FRAME_SIZE 4096
#define BITS_PER_BYTE 8

struct kernel_meminfo {
  uint32_t kernel_physical_start;
  uint32_t kernel_physical_end;
  uint32_t kernel_virtual_start;
  uint32_t kernel_virtual_end;
} __attribute__((packed));
typedef struct kernel_meminfo kernel_meminfo_t;

void init_paging(void);
void enable_paging(void);
void setup_higher_half(void);
uint32_t* create_page_directory(void);
void map_page(uint32_t* page_directory, uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags);
void unmap_page(uint32_t* page_directory, uint32_t virtual_addr);
uint32_t get_physical_address(uint32_t* page_directory, uint32_t virtual_addr);

#endif /* PAGING_H */
