#include "arch/x86/pit.h"
#include "arch/x86/interrupt.h"
#include "arch/x86/io.h"
#include "arch/x86/pic.h"
#include "lib/log.h"
#include "mem/process.h"

#define PIT_FREQUENCY 1193182

static void pit_handler(cpu_state_t s, idt_info_t i, stack_state_t e) {
  (void)s;
  (void)i;
  (void)e;

  LOG_WARN("PIT handler called");
  schedule();

  pic_acknowledge();
}

void pit_init(void) {
  uint8_t data = (1 << 5) | (1 << 4) | (1 << 2) | (1 << 1) | 0x00;
  outb(PIT_COMMAND, data);

  register_interrupt_handler(PIT_INT_IDX, pit_handler);
  pit_set_interval(10);

  LOG_INFO("PIT initialized");
  LOG_LINE();
}

void pit_set_interval(uint32_t interval) {
  uint32_t frequency = 1000 / interval;
  uint16_t divider = (uint16_t)(PIT_FREQUENCY / frequency);

  outb(PIT_CHANNEL_0_DATA, (uint8_t)divider);
  outb(PIT_CHANNEL_0_DATA, (uint8_t)(divider >> 8));

  LOG_DEBUG("PIT set to %dHz", frequency);
}
