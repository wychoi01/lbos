#ifndef STRING_H
#define STRING_H

#include <stdarg.h>
#include <stddef.h>

void* memset(void* dest, int val, size_t count);
void* memcpy(void* dest, const void* src, size_t count);
size_t strlen(const char* str);
char* strncpy(char* dest, const char* src, size_t count);
int strcmp(const char* str1, const char* str2);
int strncmp(const char* str1, const char* str2, size_t count);
char* strcpy(char* dest, const char* src);
char* strstr(const char* haystack, const char* needle);
int vsnprintf(char* buffer, size_t size, const char* format, ...);

#endif // STRING_H
