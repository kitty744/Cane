#ifndef Valen_STRING_H
#define Valen_STRING_H

#include <stdint.h>

void *memset(void *ptr, int value, uint64_t num);
void *memcpy(void *dest, const void *src, uint64_t num);
int strlen(const char *str);
int strcmp(const char *str1, const char *str2);
int strncmp(const char *str1, const char *str2, uint64_t n);
char *strchr(const char *str, int c);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, uint64_t n);

#endif
