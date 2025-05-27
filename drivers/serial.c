#include "drivers/serial.h"
#include "arch/x86/io.h"
#include <stddef.h>

static void num_to_hex(uint32_t num, char* buf) {
  const char hex_chars[] = "0123456789ABCDEF";
  int i;

  for (i = 7; i >= 0; i--) {
    buf[i] = hex_chars[num & 0xF];
    num >>= 4;
  }
  buf[8] = '\0';
}

int serial_write_hex(uint32_t num) {
  char hex_buf[9];
  num_to_hex(num, hex_buf);
  return serial_write(hex_buf, 8);
}

static void num_to_decimal(int32_t num, char* buf) {
  int i = 0;
  int is_negative = 0;

  if (num < 0) {
    is_negative = 1;
    num = -num;
  }

  if (num == 0) {
    buf[i++] = '0';
  } else {
    while (num > 0) {
      buf[i++] = '0' + (num % 10);
      num /= 10;
    }
  }

  if (is_negative) {
    buf[i++] = '-';
  }

  int start = 0;
  int end = i - 1;
  while (start < end) {
    char temp = buf[start];
    buf[start] = buf[end];
    buf[end] = temp;
    start++;
    end--;
  }

  buf[i] = '\0';
}

int serial_write_decimal(int32_t num) {
  char dec_buf[12];
  num_to_decimal(num, dec_buf);
  return serial_write(dec_buf, -1);
}

void serial_configure_line(unsigned short com) { outb(SERIAL_LINE_COMMAND_PORT(com), SERIAL_LINE_COMMAND_PORT_VALUE); }

void serial_configure_fifo(unsigned short com) { outb(SERIAL_FIFO_COMMAND_PORT(com), SERIAL_FIFO_COMMAND_PORT_VALUE); }

void serial_configure_modem(unsigned short com) {
  outb(SERIAL_MODEM_COMMAND_PORT(com), SERIAL_MODEM_COMMAND_PORT_VALUE);
}

void serial_configure_baud_rate(unsigned short com, unsigned short divisor) {
  outb(SERIAL_LINE_COMMAND_PORT(com), SERIAL_LINE_ENABLE_DLAB);
  outb(SERIAL_DATA_PORT(com), (divisor >> 8) & 0x00FF);
  outb(SERIAL_DATA_PORT(com), divisor & 0x00FF);
}

int serial_is_transmit_fifo_empty(unsigned int com) { return inb(SERIAL_LINE_STATUS_PORT(com)) & 0x20; }

int serial_write_byte(unsigned short com, unsigned char byte) {
  int attempts = 0;

  while (serial_is_transmit_fifo_empty(com) == 0) {
    attempts++;
    if (attempts >= SERIAL_MAX_WAIT_ATTEMPTS) {
      return -1;
    }
  }

  outb(SERIAL_DATA_PORT(com), byte);
  return 0;
}

static size_t strlen(const char* str) {
  size_t len = 0;
  while (str[len] != '\0') {
    len++;
  }
  return len;
}

int serial_write(const char* str, int len) {
  if (str == NULL) {
    return 0;
  }

  if (len <= 0) {
    len = strlen(str);
  }

  int i;
  for (i = 0; i < len; i++) {
    if (serial_write_byte(SERIAL_COM1_BASE, str[i]) < 0) {
      return i;
    }
  }

  int attempts = 0;
  while (serial_is_transmit_fifo_empty(SERIAL_COM1_BASE) == 0) {
    attempts++;
    if (attempts >= SERIAL_MAX_WAIT_ATTEMPTS) {
      break;
    }
  }

  return len;
}

int serial_init(unsigned short com) {
  if (com != SERIAL_COM1_BASE) {
    return -1;
  }

  serial_configure_baud_rate(com, 1);
  serial_configure_line(com);
  serial_configure_fifo(com);
  serial_configure_modem(com);

  const unsigned char test_byte = 0xAA;
  outb(SERIAL_DATA_PORT(com), test_byte);

  int attempts = 0;
  while (serial_is_transmit_fifo_empty(com) == 0) {
    attempts++;
    if (attempts >= SERIAL_MAX_WAIT_ATTEMPTS) {
      return -2;
    }
  }

  return 0;
}
