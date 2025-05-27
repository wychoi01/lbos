#include "lib/log.h"
#include "drivers/serial.h"
#include <stdarg.h>

static log_level_t current_log_level = LOG_LEVEL_DEBUG;

static const char* level_strings[] = {LOG_COLOR_DEBUG "["
                                                      "DEBUG"
                                                      "]" LOG_COLOR_RESET,
                                      LOG_COLOR_INFO "["
                                                     "INFO"
                                                     "]" LOG_COLOR_RESET,
                                      LOG_COLOR_WARN "["
                                                     "WARN"
                                                     "]" LOG_COLOR_RESET,
                                      LOG_COLOR_ERROR "["
                                                      "ERROR"
                                                      "]" LOG_COLOR_RESET,
                                      LOG_COLOR_FATAL "["
                                                      "FATAL"
                                                      "]" LOG_COLOR_RESET};

static const int level_lengths[] = {
    7, /* [DEBUG] */
    6, /* [INFO] */
    6, /* [WARN] */
    7, /* [ERROR] */
    7  /* [FATAL] */
};

void log_init(void) {
  if (serial_init(SERIAL_COM1_BASE) != 0) {
    return;
  }

  for (volatile int i = 0; i < 1000000; i++)
    ;

  serial_write("\n", 1);
  serial_write("\n", 1);
}

void log_set_level(log_level_t level) { current_log_level = level; }

void log_write(log_level_t level, const char* format, ...) {
  if (level < current_log_level) {
    return;
  }

  serial_write(level_strings[level], -1);

  int spaces = 10 - level_lengths[level];
  for (int i = 0; i < spaces; i++) {
    serial_write(" ", 1);
  }

  va_list args;
  va_start(args, format);

  const char* p = format;
  while (*p != '\0') {
    if (*p == '%') {
      p++;
      switch (*p) {
      case 'd': {
        int val = va_arg(args, int);
        serial_write_decimal(val);
        break;
      }
      case 'x': {
        unsigned int val = va_arg(args, unsigned int);
        serial_write_hex(val);
        break;
      }
      case 's': {
        char* str = va_arg(args, char*);
        if (str) {
          serial_write(str, -1);
        } else {
          serial_write("(null)", 6);
        }
        break;
      }
      case 'c': {
        char c = (char)va_arg(args, int);
        serial_write(&c, 1);
        break;
      }
      default:
        serial_write(p, 1);
      }
    } else if (*p == '\033') {
      if (level != LOG_LEVEL_INFO) {
        const char* color_start = p;
        while (*p != 'm' && *p != '\0') {
          p++;
        }
        if (*p == 'm') {
          p++;
          serial_write(color_start, p - color_start);
        }
      } else {
        while (*p != 'm' && *p != '\0') {
          p++;
        }
        if (*p == 'm') {
          p++;
        }
      }
      continue;
    } else {
      serial_write(p, 1);
    }
    p++;
  }

  va_end(args);
  serial_write("\n", 1);
}

void log_debug(const char* format, ...) {
  if (current_log_level <= LOG_LEVEL_DEBUG) {
    va_list args;
    va_start(args, format);
    log_write(LOG_LEVEL_DEBUG, format, args);
    va_end(args);
  }
}

void log_info(const char* format, ...) {
  if (current_log_level <= LOG_LEVEL_INFO) {
    va_list args;
    va_start(args, format);
    log_write(LOG_LEVEL_INFO, format, args);
    va_end(args);
  }
}

void log_warn(const char* format, ...) {
  if (current_log_level <= LOG_LEVEL_WARN) {
    va_list args;
    va_start(args, format);
    log_write(LOG_LEVEL_WARN, format, args);
    va_end(args);
  }
}

void log_error(const char* format, ...) {
  if (current_log_level <= LOG_LEVEL_ERROR) {
    va_list args;
    va_start(args, format);
    log_write(LOG_LEVEL_ERROR, format, args);
    va_end(args);
  }
}

void log_fatal(const char* format, ...) {
  if (current_log_level <= LOG_LEVEL_FATAL) {
    va_list args;
    va_start(args, format);
    log_write(LOG_LEVEL_FATAL, format, args);
    va_end(args);
  }
}
