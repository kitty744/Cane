# Device Drivers

The Device Driver system in Valen provides basic hardware support for essential input devices.

## Overview

Valen currently includes a simple driver system focused on essential kernel functionality. The primary driver implemented is the PS/2 keyboard driver, which provides input for the shell interface.

## Available Drivers

### PS/2 Keyboard Driver

The keyboard driver handles PS/2 keyboard input and converts scancodes to ASCII characters.

```c
#include <valen/keyboard.h>

// Initialize the keyboard driver
keyboard_init();

// Wait for a keypress
wait_for_keypress();

// The keyboard handler is automatically called via IRQ1
```

#### Keyboard Features

- **Scancode to ASCII conversion** - Standard US-QWERTY layout
- **Shift key support** - Handles both left and right shift keys
- **Special key handling** - Backspace, Enter, Arrow keys
- **Interrupt-driven** - Uses IRQ1 for efficient input processing

#### Key Constants

```c
#define KEY_LEFT -1    // Left arrow key
#define KEY_RIGHT -2   // Right arrow key
```

#### Keyboard Integration

The keyboard driver integrates directly with the shell system:

```c
// In keyboard_handler():
switch (scancode) {
case 0x0E:
    shell_input('\b');  // Backspace
    break;
case 0x1C:
    shell_input('\n');  // Enter
    break;
case 0x4B:
    shell_input(-1);   // Left arrow
    break;
case 0x4D:
    shell_input(-2);   // Right arrow
    break;
default:
    // Convert scancode to ASCII and send to shell
    char c = shift_pressed ? scancode_to_ascii_shift[scancode] : scancode_to_ascii[scancode];
    if (c && c != '\0') {
        shell_input(c);
    }
    break;
}
```

## Hardware Interface

### I/O Port Access

Drivers use the I/O port functions for hardware communication:

```c
#include <valen/io.h>

// Read from I/O ports
uint8_t status = inb(0x64);    // Read keyboard status
uint8_t scancode = inb(0x60);  // Read scancode

// Write to I/O ports
outb(0x64, command);  // Send command to keyboard
```

### Interrupt Handling

The keyboard driver uses the PIC (Programmable Interrupt Controller):

```c
#include <valen/pic.h>

// Enable keyboard interrupt
pic_irq_enable(IRQ_KEYBOARD);

// Send End of Interrupt signal
pic_send_eoi(IRQ_KEYBOARD);
```

## Driver Architecture

### Initialization Pattern

Most drivers in Valen follow this initialization pattern:

```c
void driver_init(void) {
    // 1. Clear any pending data
    while (inb(0x64) & 1)
        inb(0x60);

    // 2. Enable interrupt
    pic_irq_enable(IRQ_KEYBOARD);
}
```

### Interrupt Handler Pattern

```c
void driver_handler(void) {
    // 1. Check device status
    uint8_t status = inb(0x64);

    // 2. Verify data is available
    if ((status & 0x01) && !(status & 0x20)) {
        // 3. Read data
        uint8_t data = inb(0x60);

        // 4. Process data
        process_device_data(data);
    }

    // 5. Send EOI
    pic_send_eoi(IRQ_DEVICE);
}
```

## Adding New Drivers

### Basic Driver Template

To add a new driver to Valen:

1. **Create header file** (`include/valen/new_device.h`):

```c
#ifndef NEW_DEVICE_H
#define NEW_DEVICE_H

#include <stdint.h>

void new_device_init(void);
void new_device_handler(void);

#endif
```

2. **Create implementation** (`drivers/new_device.c`):

```c
#include <valen/new_device.h>
#include <valen/io.h>
#include <valen/pic.h>

void new_device_init(void) {
    // Initialize hardware
    // Enable interrupts
    pic_irq_enable(IRQ_NEW_DEVICE);
}

void new_device_handler(void) {
    // Handle device interrupts
    pic_send_eoi(IRQ_NEW_DEVICE);
}
```

3. **Update kernel initialization**:

```c
// In kernel.c:
#include <valen/new_device.h>

void kmain(unsigned long magic, unsigned long addr) {
    // ... existing initialization ...

    new_device_init();  // Add new driver init

    // ... rest of kernel ...
}
```

## Current Limitations

- **No module system** - Drivers are compiled into the kernel
- **No device discovery** - Drivers are statically initialized
- **Limited hardware support** - Only essential devices currently supported
- **No power management** - Devices are always on
- **No hot-plug support** - Devices must be present at boot

## Future Driver Development

Planned driver improvements:

1. **Modular driver system** - Loadable kernel modules
2. **Device enumeration** - Automatic hardware detection
3. **More device support** - Storage, network, graphics
4. **Power management** - Device sleep/wake states
5. **Hot-plug support** - Dynamic device addition/removal

## Integration Example

```c
#include <valen/keyboard.h>
#include <valen/stdio.h>

void input_example(void) {
    printf("Press any key to continue...\n");

    wait_for_keypress();

    printf("Key pressed! Continuing...\n");
}

void custom_input_handler(void) {
    // Custom processing can be added by modifying
    // the keyboard_handler() function in keyboard.c
}
```

This driver system provides the essential functionality needed for basic kernel operation while remaining simple and maintainable.
