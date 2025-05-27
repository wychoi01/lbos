#ifndef PAGE_FRAME_ALLOCATOR_H
#define PAGE_FRAME_ALLOCATOR_H

#include <stdbool.h>
#include <stdint.h>

void init_page_frame_allocator(uint32_t phys_start, uint32_t phys_end, uint32_t virt_start, uint32_t virt_end);
uint32_t alloc_frame(void);
void free_frame(uint32_t frame_addr);
bool is_frame_allocated(uint32_t frame_addr);

uint32_t phys_to_virt(uint32_t phys_addr);
uint32_t virt_to_phys(uint32_t virt_addr);

#endif /* PAGE_FRAME_ALLOCATOR_H */
