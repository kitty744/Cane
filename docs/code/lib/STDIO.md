# STDIO Library

The STDIO library provides VGA text mode output, serial communication, and formatted printing capabilities for Valen.

## Overview

The STDIO library is the primary interface for kernel output in Valen. It supports both VGA text mode display for user interaction and serial port output for debugging. The library includes a powerful printf implementation with extensive formatting options.

## Quick Start

```c
#include <valen/stdio.h>

void kernel_main(void) {
    // Set text color (optional)
    set_color(0x0F); // White text on black background

    // Basic output
    printf("Hello, Valen!\n");

    // Formatted output
    printf("System initialized: %d MB RAM\n", memory_size);

    // Debug output to serial
    serial_write("Debug: Kernel started successfully\n");
}
```

## VGA Text Mode Output

### Basic Printing

The VGA text mode provides an 80x25 character display with color support:

```c
// Simple text output
printf("This is printed to the VGA buffer\n");

// Character output
putc('A');

// String output
puts("Hello World");
```

### Screen Management

```c
// Clear the screen (preserves status bar at row 0)
print_clear();

// Manual cursor control
set_cursor(10, 5);  // Column 10, Row 5
int x = get_cursor_x();
int y = get_cursor_y();
```

### Color Support

Use the color definitions from "valen/color.h" for consistent color management:

```c
#include <valen/color.h>

// Basic colors
set_color(COLOR_WHITE);              // Default white text
set_color(COLOR_RED);                // Red text
set_color(COLOR_GREEN);              // Green text
set_color(COLOR_BLUE);               // Blue text
set_color(COLOR_YELLOW);             // Yellow text
set_color(COLOR_CYAN);               // Cyan text
set_color(COLOR_MAGENTA);            // Magenta text

// Background colors
set_color(COLOR_WHITE | COLOR_BG_BLACK);    // White on black
set_color(COLOR_RED | COLOR_BG_WHITE);      // Red on white
set_color(COLOR_GREEN | COLOR_BG_BLUE);      // Green on blue
set_color(COLOR_YELLOW | COLOR_BG_RED);      // Yellow on red

// Bright colors
set_color(COLOR_BRIGHT_WHITE);       // Bright white
set_color(COLOR_BRIGHT_RED);         // Bright red
set_color(COLOR_BRIGHT_GREEN);       // Bright green
set_color(COLOR_BRIGHT_BLUE);        // Bright blue

// Custom combinations
uint8_t custom_color = COLOR_GREEN | COLOR_BG_BLUE | COLOR_BRIGHT;
set_color(custom_color);
```

### Cursor Control

```c
// Enable/disable hardware cursor
enable_cursor(14, 15);  // Set cursor shape
hide_hardware_cursor();
show_hardware_cursor();

// Manual cursor positioning
set_cursor(0, 1);  // Start of user area (below status bar)
```

## Formatted Printing

The printf function supports extensive formatting options:

### Number Formatting

```c
int value = -42;
unsigned int unsigned_val = 255;
long long big_num = 1234567890LL;

printf("Signed: %d\n", value);           // -42
printf("Unsigned: %u\n", unsigned_val);   // 255
printf("Long: %ld\n", big_num);          // 1234567890
printf("Long long: %llu\n", big_num);    // 1234567890
```

### Base Conversion

```c
unsigned int val = 255;

printf("Hex: %x\n", val);      // ff
printf("HEX: %X\n", val);      // FF
printf("Octal: %o\n", val);    // 377
printf("Binary: %b\n", val);    // 11111111
```

### Pointers and Addresses

```c
void *ptr = &some_variable;
printf("Pointer: %p\n", ptr);   // 0xFFFFFFFF80001000
```

### Strings and Characters

```c
printf("String: %s\n", "Hello");
printf("Character: %c\n", 'A');
printf("Literal %%: %%\n");
```

## String to Number Conversion

The STDIO library provides string parsing functions:

```c
// Convert string to integer
int result = atoi("123");        // Returns 123
int negative = atoi("-45");       // Returns -45
int with_space = atoi("  7  ");  // Returns 7

// Handle invalid input gracefully
int invalid = atoi("abc");       // Returns 0
```

### atoi Function Details

The `atoi` function handles:

- Leading whitespace (spaces, tabs, newlines)
- Optional sign (+ or -)
- Digit sequences
- Stops at first non-digit character

```c
// Examples
atoi("123")      // 123
atoi("-45")      // -45
atoi("+67")      // 67
atoi("  89 ")    // 89
atoi("12abc34")   // 12 (stops at 'a')
atoi("abc")       // 0
atoi("")          // 0
```

## Serial Communication

Serial output is primarily used for debugging and diagnostics:

```c
// Basic serial output
serial_write("Kernel boot started\n");

// Formatted serial output
serial_write_int(memory_size);
serial_write_hex(0xDEADBEEF);
```

## Error Handling

The STDIO library is designed to be robust:

- All VGA operations use spinlocks for thread safety
- Serial operations are protected against concurrent access
- Invalid format specifiers are handled gracefully
- Cursor operations are bounded to screen dimensions

## Best Practices

1. **Use serial output for debugging** - Serial works even when VGA is unavailable
2. **Preserve the status bar** - Row 0 is reserved for system status
3. **Use appropriate colors** - Maintain visual consistency
4. **Handle thread safety** - The library handles locking automatically
5. **Format strings carefully** - Use proper format specifiers

## Integration Example

```c
#include <valen/stdio.h>
#include <valen/color.h>

void init_display(void) {
    // Initialize display system
    set_color(COLOR_WHITE | COLOR_BG_BLACK);
    print_clear();

    // Set up status bar
    set_cursor(0, 0);
    set_color(COLOR_BLACK | COLOR_BG_LIGHT_GREY);
    printf("Valen v1.0 - Ready");

    // Reset to user area
    set_color(COLOR_WHITE | COLOR_BG_BLACK);
    set_cursor(0, 1);
}

void kernel_main(void) {
    init_display();

    printf("System initialization complete\n");
    printf("Memory: %d MB available\n", get_memory_size());

    // Color-coded status messages
    set_color(COLOR_GREEN);
    printf("[OK] Kernel initialized\n");

    set_color(COLOR_YELLOW);
    printf("[WARN] Debug mode enabled\n");

    set_color(COLOR_WHITE);  // Reset to default
    serial_write("Kernel initialization successful\n");
}
```

## Thread Safety

All STDIO functions that modify shared state (VGA buffer, cursor position, serial port) use spinlocks to ensure thread safety in multi-kernel-thread environments. The locking is handled automatically - no additional synchronization is required from the caller.

## Constants and Configuration

- **VGA Buffer Address**: `0xFFFFFFFF800B8000` (Higher Half Virtual Address)
- **Screen Dimensions**: 80 columns × 25 rows
- **Default Color**: `0x0F` (White text on black background)
- **Serial Port**: COM1 (0x3f8)
