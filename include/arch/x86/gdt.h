#ifndef GDT_H
#define GDT_H

#include <stdint.h>

struct gdt_entry {
  uint16_t limit_low;
  uint16_t base_low;
  uint8_t base_middle;
  uint8_t access;
  uint8_t granularity;
  uint8_t base_high;
} __attribute__((packed));
typedef struct gdt_entry gdt_entry_t;

struct gdt_ptr {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed));
typedef struct gdt_ptr gdt_ptr_t;

extern gdt_entry_t gdt_entries[6];
extern gdt_ptr_t gdt_ptr;

#define GDT_NULL_INDEX 0
#define GDT_CODE_INDEX 1
#define GDT_DATA_INDEX 2
#define GDT_USER_CODE_INDEX 3
#define GDT_USER_DATA_INDEX 4
#define GDT_TSS_INDEX 5

#define GDT_NULL_OFFSET 0x00
#define GDT_CODE_OFFSET 0x08
#define GDT_DATA_OFFSET 0x10
#define GDT_USER_CODE_OFFSET 0x18
#define GDT_USER_DATA_OFFSET 0x20
#define GDT_TSS_OFFSET 0x28

#define GDT_ACCESS_PRESENT 0x80
#define GDT_ACCESS_RING0 0x00
#define GDT_ACCESS_RING1 0x20
#define GDT_ACCESS_RING2 0x40
#define GDT_ACCESS_RING3 0x60
#define GDT_ACCESS_CODE 0x18
#define GDT_ACCESS_DATA 0x10
#define GDT_ACCESS_EXECUTABLE 0x08
#define GDT_ACCESS_READABLE 0x02
#define GDT_ACCESS_WRITABLE 0x02

#define GDT_GRAN_4KB 0x80
#define GDT_GRAN_32BIT 0x40

void gdt_init(void);
void gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran);
extern void gdt_asm(void);

#endif /* GDT_H */
