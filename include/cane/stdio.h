#ifndef CANE_STDIO_H
#define CANE_STDIO_H

#include <stdarg.h>
#include <stdint.h>

void set_color(uint8_t color);
void printf(const char *format, ...);
void print_int(int num);
void print_uint(uint64_t num);
void print_hex(uint64_t num);
void print_hex_upper(uint64_t num);
void print_octal(uint64_t num);
void print_binary(uint64_t num);
void puts(const char *str);
void putchar(char c);
void clear_screen(void);
void set_cursor(uint8_t x, uint8_t y);

#endif
