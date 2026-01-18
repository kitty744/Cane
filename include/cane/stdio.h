#ifndef CANE_STDIO_H
#define CANE_STDIO_H

#include <stdarg.h>
#include <stdint.h>

void printf(const char *format, ...);
void puts(const char *str);
void putchar(char c);
void clear_screen(void);
void set_cursor(uint8_t x, uint8_t y);

#endif
