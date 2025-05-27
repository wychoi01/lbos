#include "arch/x86/gdt.h"
#include "lib/log.h"

gdt_entry_t gdt_entries[6];
gdt_ptr_t gdt_ptr;

void gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran) {
  gdt_entries[num].base_low = (base & 0xFFFF);
  gdt_entries[num].base_middle = (base >> 16) & 0xFF;
  gdt_entries[num].base_high = (base >> 24) & 0xFF;

  gdt_entries[num].limit_low = (limit & 0xFFFF);
  gdt_entries[num].granularity = ((limit >> 16) & 0x0F);
  gdt_entries[num].granularity |= (gran & 0xF0);
  gdt_entries[num].access = access;

  LOG_DEBUG("GDT Entry 0x%x:", num);
  LOG_DEBUG("\tBase: 0x%x (low: 0x%x, middle: 0x%x, high: 0x%x)", base, gdt_entries[num].base_low,
            gdt_entries[num].base_middle, gdt_entries[num].base_high);
  LOG_DEBUG("\tLimit: 0x%x (low: 0x%x, high: 0x%x)", limit, gdt_entries[num].limit_low,
            gdt_entries[num].granularity & 0x0F);

  LOG_DEBUG("\tAccess Byte (0x%x):", access);
  LOG_DEBUG("\t\tPresent: %s", (access & GDT_ACCESS_PRESENT) ? "Yes" : "No");
  LOG_DEBUG("\t\tPrivilege Level: 0x%x", (access & 0x60) >> 5);
  LOG_DEBUG("\t\tType: %s", (access & GDT_ACCESS_CODE) ? "Code" : "Data");
  LOG_DEBUG("\t\tExecutable: %s", (access & GDT_ACCESS_EXECUTABLE) ? "Yes" : "No");
  LOG_DEBUG("\t\tDirection/Conforming: %s", (access & 0x04) ? "Down/Conforming" : "Up/Non-conforming");
  LOG_DEBUG("\t\tReadable/Writable: %s", (access & 0x02) ? "Yes" : "No");
  LOG_DEBUG("\t\tAccessed: %s", (access & 0x01) ? "Yes" : "No");

  LOG_DEBUG("\tGranularity Byte (0x%x):", gran);
  LOG_DEBUG("\t\tPage Granularity: %s", (gran & GDT_GRAN_4KB) ? "4KB" : "1B");
  LOG_DEBUG("\t\tSize: %s", (gran & GDT_GRAN_32BIT) ? "32-bit" : "16-bit");
  LOG_DEBUG("\t\tAvailable: %s", (gran & 0x10) ? "Yes" : "No");
  LOG_DEBUG("\t\tLong Mode: %s", (gran & 0x20) ? "Yes" : "No");
}

void gdt_init(void) {
  gdt_ptr.limit = (sizeof(gdt_entry_t) * 6) - 1;
  gdt_ptr.base = (unsigned int)&gdt_entries;

  LOG_INFO("Initializing GDT with %d entries", 6);
  LOG_INFO("GDT Base: 0x%x, Limit: 0x%x", gdt_ptr.base, gdt_ptr.limit);

  LOG_INFO("Setting up NULL descriptor");
  gdt_set_gate(GDT_NULL_INDEX, 0, 0, 0, 0);

  LOG_INFO("Setting up code segment descriptor");
  gdt_set_gate(GDT_CODE_INDEX, 0, 0xFFFFFFFF,
               GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_CODE | GDT_ACCESS_EXECUTABLE | GDT_ACCESS_READABLE,
               GDT_GRAN_4KB | GDT_GRAN_32BIT);

  LOG_INFO("Setting up data segment descriptor");
  gdt_set_gate(GDT_DATA_INDEX, 0, 0xFFFFFFFF,
               GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_DATA | GDT_ACCESS_WRITABLE,
               GDT_GRAN_4KB | GDT_GRAN_32BIT);

  LOG_INFO("Setting up user code segment descriptor");
  gdt_set_gate(GDT_USER_CODE_INDEX, 0, 0xFFFFFFFF,
               GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_CODE | GDT_ACCESS_EXECUTABLE | GDT_ACCESS_READABLE,
               GDT_GRAN_4KB | GDT_GRAN_32BIT);

  LOG_INFO("Setting up user data segment descriptor");
  gdt_set_gate(GDT_USER_DATA_INDEX, 0, 0xFFFFFFFF,
               GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_DATA | GDT_ACCESS_WRITABLE,
               GDT_GRAN_4KB | GDT_GRAN_32BIT);

  gdt_asm();

  LOG_INFO("GDT initialized");
  LOG_LINE();
}
