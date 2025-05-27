#ifndef PIT_H
#define PIT_H

#include <stdint.h>

#define PIT_CHANNEL_0_DATA 0x40
#define PIT_CHANNEL_1_DATA 0x41
#define PIT_CHANNEL_2_DATA 0x42
#define PIT_COMMAND 0x43

#define PIT_CMD_CHANNEL0 0x00
#define PIT_CMD_LOBYTE 0x10
#define PIT_CMD_HIBYTE 0x20
#define PIT_CMD_SQUARE 0x06
#define PIT_CMD_INTERRUPT 0x0E

#define PIT_FREQUENCY 1193182

void pit_init(void);
void pit_set_interval(uint32_t interval);

#endif /* PIT_H */
