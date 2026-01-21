/**
 * @file stdio.c
 * @brief Standard I/O library for VGA text mode and Serial output.
 */

#include "Valen/stdio.h"
#include "Valen/io.h"
#include "Valen/spinlock.h"

/* Higher Half Virtual Address for VGA Buffer */
#define VIRT_ADDR 0xFFFFFFFF800B8000

static uint16_t *buffer = (uint16_t *)VIRT_ADDR;
static int cursor_x = 0;
static int cursor_y = 0;
const int width = 80;
const int height = 25;
static uint8_t terminal_attribute = 0x0F;

static spinlock_t lock = {0};

/**
 * @brief Sets the global text color for kprint.
 */
void set_color(uint8_t color)
{
    terminal_attribute = color;
}

/**
 * @brief Writes a string to COM1 Serial Port for diagnostics.
 * I/O ports (outb) do not change in the Higher Half.
 */
void serial_write(char *s)
{
    spinlock_init(&lock);
    while (*s)
    {
        outb(0x3f8, *s++);
    }
    spinlock_release(&lock);
}

/**
 * @brief Sends an integer to the serial port.
 */
void serial_write_int(uint64_t n)
{
    if (n == 0)
    {
        serial_write("0");
        return;
    }
    char buf[21];
    int i = 19;
    buf[20] = '\0';
    while (n > 0)
    {
        buf[i--] = (n % 10) + '0';
        n /= 10;
    }
    serial_write(&buf[i + 1]);
}

int get_cursor_x() { return cursor_x; }
int get_cursor_y() { return cursor_y; }

/**
 * @brief Communicates with the VGA hardware to move the blinking cursor.
 */
void update_cursor(int x, int y)
{
    uint16_t pos = y * width + x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

/**
 * @brief Manually sets cursor position with spinlock protection.
 */
void set_cursor(int x, int y)
{
    spinlock_init(&lock);
    cursor_x = x;
    cursor_y = y;
    update_cursor(cursor_x, cursor_y);
    spinlock_release(&lock);
}

/**
 * @brief Configures hardware cursor shape.
 */
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end)
{
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | cursor_start);
    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | cursor_end);
}

/**
 * @brief Clears the screen EXCEPT for the status bar (Row 0).
 */
void print_clear()
{
    spinlock_init(&lock);
    uint16_t blank = (uint16_t)' ' | ((uint16_t)terminal_attribute << 8);
    for (int i = 0; i < width * height; i++)
    {
        buffer[i] = blank;
    }

    cursor_x = 0;
    cursor_y = 1;
    update_cursor(cursor_x, cursor_y);
    enable_cursor(14, 15); // Enable hardware cursor
    spinlock_release(&lock);
}

/**
 * @brief Scrolls the screen up, preserving the status bar.
 */
void print_newline()
{
    cursor_x = 0;
    if (cursor_y < height - 1)
    {
        cursor_y++;
    }
    else
    {
        /* Move all rows from row 1 to height-1 UP by one */
        for (int y = 1; y < height - 1; y++)
        {
            for (int x = 0; x < width; x++)
            {
                buffer[y * width + x] = buffer[(y + 1) * width + x];
            }
        }
        /* Clear the bottom-most row only */
        uint16_t blank = (uint16_t)' ' | ((uint16_t)terminal_attribute << 8);
        for (int x = 0; x < width; x++)
        {
            buffer[(height - 1) * width + x] = blank;
        }
        cursor_y = height - 1;
    }
    update_cursor(cursor_x, cursor_y);
}

void puts(const char *str)
{
    while (*str)
    {
        putc(*str++);
    }
}

/**
 * @brief Prints a single character. Handles wrapping and scrolling.
 */
void putc(char c)
{
    spinlock_init(&lock);
    if (c == '\n')
    {
        spinlock_release(&lock);
        print_newline();
        return;
    }

    if (cursor_x >= width)
    {
        spinlock_release(&lock);
        print_newline();
        spinlock_init(&lock);
    }

    uint8_t uc = (uint8_t)c;
    buffer[cursor_y * width + cursor_x] = (uint16_t)uc | ((uint16_t)terminal_attribute << 8);

    cursor_x++;
    update_cursor(cursor_x, cursor_y);
    spinlock_release(&lock);
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
                        print_hex(va_arg(args, unsigned long));
                    }
                }
                else
                {
                    print_hex(va_arg(args, unsigned long));
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
                putc('0');
                putc('x');
                print_hex_upper((uint64_t)va_arg(args, void *));
                break;
            case 's':
                puts(va_arg(args, char *));
                break;
            case 'c':
                putc(va_arg(args, int));
                break;
            case '%':
                putc('%');
                break;
            default:
                putc('%');
                putc(*format);
                break;
            }
        }
        else
        {
            putc(*format);
        }
        format++;
    }

    va_end(args);
}

void print_uint(uint64_t num)
{
    if (num == 0)
    {
        putc('0');
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
        putc(buffer[--i]);
    }
}

void print_int(uint64_t n)
{
    if (n == 0)
    {
        putc('0');
        return;
    }
    char buf[21];
    int i = 19;
    buf[20] = '\0';
    while (n > 0)
    {
        buf[i--] = (n % 10) + '0';
        n /= 10;
    }
    printf(&buf[i + 1]);
}

void print_hex(uint64_t n)
{
    char *chars = "0123456789ABCDEF";
    char buf[19];
    buf[18] = '\0';
    for (int i = 17; i >= 0; i--)
    {
        buf[i] = chars[n & 0xF];
        n >>= 4;
    }
    printf("0x");
    printf(buf);
}

void print_hex_upper(uint64_t num)
{
    if (num == 0)
    {
        putc('0');
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
        putc(buffer[--i]);
    }
}

void print_octal(uint64_t num)
{
    if (num == 0)
    {
        putc('0');
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
        putc(buffer[--i]);
    }
}

void print_binary(uint64_t num)
{
    if (num == 0)
    {
        putc('0');
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
        putc(buffer[--i]);
    }
}


void serial_write_hex(uint32_t n)
{
    char hex[] = "0123456789ABCDEF";
    char buffer[9] = "00000000";
    for (int i = 7; i >= 0; i--)
    {
        buffer[i] = hex[n & 0xF];
        n >>= 4;
    }
    serial_write("0x");
    serial_write(buffer);
}

/**
 * @brief Disables the hardware cursor rendering.
 * * Communicates with the CRT Controller (CRTC) registers. Setting bit 5
 * of the Cursor Start Register (0x0A) instructs the VGA hardware to
 * stop rendering the blinking cursor.
 */
void hide_hardware_cursor()
{
    outb(0x3D4, 0x0A);
    outb(0x3D5, inb(0x3D5) | 0x20);
}

/**
 * @brief Enables the hardware cursor rendering.
 * * Clears bit 5 of the Cursor Start Register (0x0A) to allow the
 * VGA hardware to render the blinking cursor at the current register position.
 */
void show_hardware_cursor()
{
    outb(0x3D4, 0x0A);
    outb(0x3D5, inb(0x3D5) & ~0x20);
}

void print_backspace()
{
    spinlock_init(&lock);
    if (cursor_x > 0)
    {
        cursor_x--;
    }
    else if (cursor_y > 1)
    { // Prevents backspacing into Row 0
        cursor_y--;
        cursor_x = width - 1;
    }
    buffer[cursor_y * width + cursor_x] = (uint16_t)' ' | ((uint16_t)terminal_attribute << 8);
    update_cursor(cursor_x, cursor_y);
    spinlock_release(&lock);
}