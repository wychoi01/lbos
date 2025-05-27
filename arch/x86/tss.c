#include "arch/x86/tss.h"
#include "arch/x86/gdt.h"
#include "lib/log.h"
#include "lib/string.h"

static tss_t tss;

extern void tss_flush(void);

void tss_init(void) {
    LOG_INFO("Initializing TSS...");

    memset(&tss, 0, sizeof(tss_t));

    tss.ss0 = GDT_DATA_OFFSET;
    tss.esp0 = 0;

    tss.iomap_base = sizeof(tss_t);

    uint32_t tss_base = (uint32_t)&tss;
    uint32_t tss_limit = sizeof(tss_t) - 1;

    gdt_set_gate(5, tss_base, tss_limit, 0x89, 0x00);

    asm volatile("ltr %%ax" :: "a"(0x28));

    LOG_INFO("TSS initialized at 0x%x", tss_base);
}

void tss_set_kernel_stack(uint32_t stack) {
    tss.esp0 = stack;
}
