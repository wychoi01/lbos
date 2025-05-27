#include "arch/x86/pic.h"
#include "arch/x86/io.h"
#include "lib/log.h"

#define PIC1_PORT_A 0x20
#define PIC1_PORT_B 0x21

#define PIC2_PORT_A 0xA0
#define PIC2_PORT_B 0xA1

#define PIC1_ICW1 0x11 /* Initialize the PIC and enable ICW4 */
#define PIC2_ICW1 PIC1_ICW2

#define PIC1_ICW2 0x20 /* IRQ 0-7 will be remapped to IDT index 32 - 39 */
#define PIC2_ICW2 0x28 /* IRQ 8-15 will be remapped to IDT index 40 - 47 */

#define PIC1_ICW3 0x04 /* PIC1 is connected to PIC2 via IRQ2 */
#define PIC2_ICW3 0x02 /* PIC2 is connected to PIC1 via IRQ1 */

#define PIC1_ICW4 0x05 /* 8086/88 mode is enabled and PIC1 is master */
#define PIC2_ICW4 0x01 /* 8086/88 mode is enabled */

#define PIC_EOI 0x20

void pic_init(void) {
  LOG_INFO("Initializing PIC");

  /* ICW1 */
  LOG_DEBUG("\tSetting up PIC1 ICW1");
  outb(PIC1_PORT_A, PIC1_ICW1);
  LOG_DEBUG("\tSetting up PIC2 ICW1");
  outb(PIC2_PORT_A, PIC2_ICW1);

  /* ICW2 */
  LOG_DEBUG("\tSetting up PIC1 ICW2");
  outb(PIC1_PORT_B, PIC1_ICW2);
  LOG_DEBUG("\tSetting up PIC2 ICW2");
  outb(PIC2_PORT_B, PIC2_ICW2);

  /* ICW3 */
  LOG_DEBUG("\tSetting up PIC1 ICW3");
  outb(PIC1_PORT_B, PIC1_ICW3);
  LOG_DEBUG("\tSetting up PIC2 ICW3");
  outb(PIC2_PORT_B, PIC2_ICW3);

  /* ICW4 */
  LOG_DEBUG("\tSetting up PIC1 ICW4");
  outb(PIC1_PORT_B, PIC1_ICW4);
  LOG_DEBUG("\tSetting up PIC2 ICW4");
  outb(PIC2_PORT_B, PIC2_ICW4);

  LOG_DEBUG("\tMasking PIC1 and PIC2");
  pic_mask(0xEC, 0xFF);

  LOG_INFO("PIC initialized");
  LOG_LINE();
}

void pic_acknowledge() {
  outb(PIC1_PORT_A, PIC_EOI);
  outb(PIC2_PORT_A, PIC_EOI);
}

void pic_mask(uint8_t mask1, uint8_t mask2) {
  outb(PIC1_PORT_B, mask1);
  outb(PIC2_PORT_B, mask2);
}
