#ifndef CANE_STRING_H
#define CANE_STRING_H

#include <stdint.h>

void *memset(void *ptr, int value, uint64_t num);
void *memcpy(void *dest, const void *src, uint64_t num);
int strlen(const char *str);
int strcmp(const char *str1, const char *str2);

#endif
