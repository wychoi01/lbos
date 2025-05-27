#ifndef INCLUDE_LIB_LOG_H
#define INCLUDE_LIB_LOG_H

#include "drivers/serial.h"
#include <stddef.h>
#include <stdint.h>

#define LOG_COLOR_RESET "\033[0m"
#define LOG_COLOR_DEBUG "\033[36m"
#define LOG_COLOR_INFO "\033[32m"
#define LOG_COLOR_WARN "\033[33m"
#define LOG_COLOR_ERROR "\033[31m"
#define LOG_COLOR_FATAL "\033[35m"
#define LOG_COLOR_BOOT "\033[1;36m"

typedef enum { LOG_LEVEL_DEBUG = 0, LOG_LEVEL_INFO, LOG_LEVEL_WARN, LOG_LEVEL_ERROR, LOG_LEVEL_FATAL } log_level_t;

void log_init(void);
void log_set_level(log_level_t level);

#define LOG_LINE() serial_write("\n", 1)
#define LOG_DEBUG(fmt, ...) log_write(LOG_LEVEL_DEBUG, LOG_COLOR_DEBUG fmt LOG_COLOR_RESET, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) log_write(LOG_LEVEL_INFO, LOG_COLOR_INFO fmt LOG_COLOR_RESET, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) log_write(LOG_LEVEL_WARN, LOG_COLOR_WARN fmt LOG_COLOR_RESET, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) log_write(LOG_LEVEL_ERROR, LOG_COLOR_ERROR fmt LOG_COLOR_RESET, ##__VA_ARGS__)
#define LOG_FATAL(fmt, ...) log_write(LOG_LEVEL_FATAL, LOG_COLOR_FATAL fmt LOG_COLOR_RESET, ##__VA_ARGS__)

void log_write(log_level_t level, const char* format, ...);

#endif /* INCLUDE_LIB_LOG_H */
