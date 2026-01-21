#ifndef STDIO_H
#define STDIO_H

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#define KEY_LEFT -1
#define KEY_RIGHT -2

extern const int width;
extern const int height;

void printf(const char *format, ...);
void puts(const char *str);
void putc(char c);
void print_clear();
void print_int(uint64_t n);
void print_hex(uint64_t n);
void print_uint(uint64_t num);
void print_hex_upper(uint64_t num);
void print_octal(uint64_t num);
void print_binary(uint64_t num);
void print_backspace();
void print_newline();

// String to number conversion
int atoi(const char *str);

void serial_write(char *s);
void serial_write_int(uint64_t n);
void serial_write_hex(uint32_t n);

void update_cursor(int x, int y);
void set_cursor(int x, int y);
int get_cursor_x();
int get_cursor_y();

void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
void hide_hardware_cursor();
void show_hardware_cursor();

void set_color(uint8_t color);

#endif