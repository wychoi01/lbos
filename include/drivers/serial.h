#ifndef INCLUDE_SERIAL_H
#define INCLUDE_SERIAL_H

#include "arch/x86/io.h"
#include <stddef.h>
#include <stdint.h>

#define SERIAL_COM1_BASE 0x3F8

#define SERIAL_DATA_PORT(base) (base)
#define SERIAL_FIFO_COMMAND_PORT(base) (base + 2)
#define SERIAL_LINE_COMMAND_PORT(base) (base + 3)
#define SERIAL_MODEM_COMMAND_PORT(base) (base + 4)
#define SERIAL_LINE_STATUS_PORT(base) (base + 5)

#define SERIAL_LINE_ENABLE_DLAB 0x80
#define SERIAL_LINE_COMMAND_PORT_VALUE 0x03
#define SERIAL_FIFO_COMMAND_PORT_VALUE 0xC7
#define SERIAL_MODEM_COMMAND_PORT_VALUE 0x03
#define SERIAL_MAX_WAIT_ATTEMPTS 1000

void serial_configure_baud_rate(unsigned short com, unsigned short divisor);
void serial_configure_line(unsigned short com);
void serial_configure_fifo(unsigned short com);
void serial_configure_modem(unsigned short com);
int serial_is_transmit_fifo_empty(unsigned int com);

int serial_write_byte(unsigned short com, unsigned char byte);
int serial_write(const char* str, int len);
int serial_write_hex(uint32_t num);
int serial_write_decimal(int32_t num);

int serial_init(unsigned short com);

#endif /* INCLUDE_SERIAL_H */
