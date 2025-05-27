#include "lib/string.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

void* memset(void* dest, int val, size_t count) {
  unsigned char* ptr = (unsigned char*)dest;
  while (count-- > 0) {
    *ptr++ = val;
  }
  return dest;
}

void* memcpy(void* dest, const void* src, size_t count) {
  unsigned char* d = (unsigned char*)dest;
  const unsigned char* s = (const unsigned char*)src;
  while (count-- > 0) {
    *d++ = *s++;
  }
  return dest;
}

size_t strlen(const char* str) {
  size_t len = 0;
  while (str[len]) {
    len++;
  }
  return len;
}

char* strcpy(char* dest, const char* src) {
  char* d = dest;
  while ((*d++ = *src++));
  return dest;
}

char* strncpy(char* dest, const char* src, size_t count) {
  char* d = dest;
  const char* s = src;

  while (count > 0 && *s != '\0') {
    *d++ = *s++;
    count--;
  }

  while (count > 0) {
    *d++ = '\0';
    count--;
  }

  return dest;
}

int strcmp(const char* str1, const char* str2) {
  while (*str1 && (*str1 == *str2)) {
    str1++;
    str2++;
  }
  return *(unsigned char*)str1 - *(unsigned char*)str2;
}

int strncmp(const char* str1, const char* str2, size_t count) {
  while (count > 0 && *str1 && (*str1 == *str2)) {
    str1++;
    str2++;
    count--;
  }
  if (count == 0) return 0;
  return *(unsigned char*)str1 - *(unsigned char*)str2;
}

char* strstr(const char* haystack, const char* needle) {
  if (*needle == '\0') {
    return (char*)haystack;
  }

  while (*haystack) {
    if (*haystack == *needle) {
      const char* h = haystack;
      const char* n = needle;

      while (*h && *n && *h == *n) {
        h++;
        n++;
      }

      if (*n == '\0') {
        return (char*)haystack;
      }
    }

    haystack++;
  }

  return NULL;
}

static void number_to_string(char* buffer, size_t* pos, int num, int base) {
    char num_buf[32];
    int num_pos = 0;
    int is_neg = 0;
    unsigned int unum;

    if (num < 0 && base == 10) {
        is_neg = 1;
        unum = -num;
    } else {
        unum = num;
    }

    do {
        int digit = unum % base;
        num_buf[num_pos++] = (digit < 10) ? '0' + digit : 'a' + (digit - 10);
        unum /= base;
    } while (unum > 0);

    if (is_neg) {
        num_buf[num_pos++] = '-';
    }

    while (num_pos > 0) {
        buffer[(*pos)++] = num_buf[--num_pos];
    }
}

static void pointer_to_string(char* buffer, size_t* pos, void* ptr) {
    uintptr_t addr = (uintptr_t)ptr;
    buffer[(*pos)++] = '0';
    buffer[(*pos)++] = 'x';

    for (int i = sizeof(void*) * 2 - 1; i >= 0; i--) {
        int digit = (addr >> (i * 4)) & 0xF;
        buffer[(*pos)++] = (digit < 10) ? '0' + digit : 'a' + (digit - 10);
    }
}

int vsnprintf(char* buffer, size_t size, const char* format, ...) {
    size_t pos = 0;
    const char* fmt = format;
    va_list args;
    va_start(args, format);

    while (*fmt && pos < size - 1) {
        if (*fmt == '%' && *(fmt + 1) != '%') {
            fmt++;
            switch (*fmt) {
                case 'd':
                case 'i':
                    number_to_string(buffer, &pos, va_arg(args, int), 10);
                    break;

                case 'u':
                    number_to_string(buffer, &pos, va_arg(args, unsigned int), 10);
                    break;

                case 'x':
                case 'X':
                    number_to_string(buffer, &pos, va_arg(args, int), 16);
                    break;

                case 'p':
                    pointer_to_string(buffer, &pos, va_arg(args, void*));
                    break;

                case 's': {
                    const char* str = va_arg(args, const char*);
                    if (!str) str = "(null)";
                    while (*str && pos < size - 1) {
                        buffer[pos++] = *str++;
                    }
                    break;
                }

                case 'c': {
                    char c = (char)va_arg(args, int);
                    buffer[pos++] = c;
                    break;
                }

                case '%':
                    buffer[pos++] = '%';
                    break;

                default:
                    buffer[pos++] = '%';
                    buffer[pos++] = *fmt;
                    break;
            }
        } else {
            buffer[pos++] = *fmt;
        }
        fmt++;
    }

    va_end(args);

    buffer[pos] = '\0';
    return (int)pos;
}
