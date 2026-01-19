#include <cane/stdio.h>
#include <cane/io.h>
#include <cane/string.h>
#include <stdint.h>

static uint8_t cursor_x = 0;
static uint8_t cursor_y = 0;
static uint8_t terminal_attribute = 0x0F;

void set_color(uint8_t color)
{
    terminal_attribute = color;
}

static void update_cursor(void)
{
    uint16_t pos = cursor_y * 80 + cursor_x;
    outb(0x3D4, 14);
    outb(0x3D5, pos >> 8);
    outb(0x3D4, 15);
    outb(0x3D5, pos & 0xFF);
}

void set_cursor(uint8_t x, uint8_t y)
{
    cursor_x = x;
    cursor_y = y;
    update_cursor();
}

void clear_screen(void)
{
    volatile uint16_t *video = (volatile uint16_t *)0xB8000;
    for (int i = 0; i < 80 * 25; i++)
    {
        video[i] = (terminal_attribute << 8) | ' ';
    }
    cursor_x = 0;
    cursor_y = 0;
    update_cursor();
}

void putchar(char c)
{
    volatile uint16_t *video = (volatile uint16_t *)0xB8000;

    switch (c)
    {
    case '\n':
        cursor_x = 0;
        cursor_y++;
        break;
    case '\r':
        cursor_x = 0;
        break;
    case '\t':
        cursor_x = (cursor_x + 8) & ~7;
        break;
    default:
        video[cursor_y * 80 + cursor_x] = (terminal_attribute << 8) | c;
        cursor_x++;
        break;
    }

    if (cursor_x >= 80)
    {
        cursor_x = 0;
        cursor_y++;
    }

    if (cursor_y >= 25)
    {
        cursor_y = 24;
        cursor_x = 0;
    }

    update_cursor();
}

void puts(const char *str)
{
    while (*str)
    {
        putchar(*str++);
    }
}

void print_int(int num)
{
    if (num == 0)
    {
        putchar('0');
        return;
    }

    char buffer[32];
    int i = 0;

    if (num < 0)
    {
        putchar('-');
        num = -num;
    }

    while (num > 0)
    {
        buffer[i++] = '0' + (num % 10);
        num /= 10;
    }

    while (i > 0)
    {
        putchar(buffer[--i]);
    }
}

void print_uint(uint64_t num)
{
    if (num == 0)
    {
        putchar('0');
        return;
    }

    char buffer[32];
    int i = 0;

    while (num > 0)
    {
        buffer[i++] = '0' + (num % 10);
        num /= 10;
    }

    while (i > 0)
    {
        putchar(buffer[--i]);
    }
}

void print_hex(uint64_t num)
{
    if (num == 0)
    {
        putchar('0');
        return;
    }

    char buffer[32];
    int i = 0;

    while (num > 0)
    {
        int digit = num % 16;
        buffer[i++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
        num /= 16;
    }

    while (i > 0)
    {
        putchar(buffer[--i]);
    }
}

void print_hex_upper(uint64_t num)
{
    if (num == 0)
    {
        putchar('0');
        return;
    }

    char buffer[32];
    int i = 0;

    while (num > 0)
    {
        int digit = num % 16;
        buffer[i++] = (digit < 10) ? ('0' + digit) : ('A' + digit - 10);
        num /= 16;
    }

    while (i > 0)
    {
        putchar(buffer[--i]);
    }
}

void print_octal(uint64_t num)
{
    if (num == 0)
    {
        putchar('0');
        return;
    }

    char buffer[32];
    int i = 0;

    while (num > 0)
    {
        buffer[i++] = '0' + (num % 8);
        num /= 8;
    }

    while (i > 0)
    {
        putchar(buffer[--i]);
    }
}

void print_binary(uint64_t num)
{
    if (num == 0)
    {
        putchar('0');
        return;
    }

    char buffer[64];
    int i = 0;

    while (num > 0)
    {
        buffer[i++] = '0' + (num % 2);
        num /= 2;
    }

    while (i > 0)
    {
        putchar(buffer[--i]);
    }
}

void printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    while (*format)
    {
        if (*format == '%' && *(format + 1))
        {
            format++;
            switch (*format)
            {
            case 'd':
            case 'i':
                print_int(va_arg(args, int));
                break;
            case 'u':
                print_uint(va_arg(args, unsigned int));
                break;
            case 'l':
                if (*(format + 1) == 'l')
                {
                    format++;
                    if (*(format + 1) == 'u')
                    {
                        format++;
                        print_uint(va_arg(args, unsigned long long));
                    }
                    else if (*(format + 1) == 'd' || *(format + 1) == 'i')
                    {
                        format++;
                        print_int(va_arg(args, long long));
                    }
                    else if (*(format + 1) == 'x')
                    {
                        format++;
                        print_hex(va_arg(args, unsigned long long));
                    }
                    else if (*(format + 1) == 'X')
                    {
                        format++;
                        print_hex_upper(va_arg(args, unsigned long long));
                    }
                    else
                    {
                        print_uint(va_arg(args, unsigned long));
                    }
                }
                else
                {
                    print_uint(va_arg(args, unsigned long));
                }
                break;
            case 'x':
                print_hex(va_arg(args, unsigned int));
                break;
            case 'X':
                print_hex_upper(va_arg(args, unsigned int));
                break;
            case 'o':
                print_octal(va_arg(args, unsigned int));
                break;
            case 'b':
                print_binary(va_arg(args, unsigned int));
                break;
            case 'p':
                putchar('0');
                putchar('x');
                print_hex_upper((uint64_t)va_arg(args, void *));
                break;
            case 's':
                puts(va_arg(args, char *));
                break;
            case 'c':
                putchar(va_arg(args, int));
                break;
            case '%':
                putchar('%');
                break;
            default:
                putchar('%');
                putchar(*format);
                break;
            }
        }
        else
        {
            putchar(*format);
        }
        format++;
    }

    va_end(args);
}
