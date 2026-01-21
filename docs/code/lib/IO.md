# I/O Library

The I/O library provides low-level hardware port access for device communication in ValenOS.

## Overview

The I/O library enables direct communication with hardware devices through port-mapped I/O operations. This is essential for interacting with VGA controllers, serial ports, keyboard controllers, and other legacy hardware interfaces.

## Quick Start

```c
#include "Valen/io.h"

void init_vga(void) {
    // Read VGA controller state
    uint8_t current = inb(0x3D4);

    // Write new cursor position
    outb(0x3D4, 0x0F);  // Select cursor low register
    outb(0x3D5, 0x00);  // Set low byte
}

void read_keyboard_status(void) {
    // Check if keyboard has data available
    uint8_t status = inb(0x64);
    if (status & 0x01) {
        uint8_t scancode = inb(0x60);
        // Process scancode
    }
}
```

## Basic I/O Operations

### Port Input

Read data from hardware ports:

```c
// 8-bit input
uint8_t byte_value = inb(port_address);

// 16-bit input
uint16_t word_value = inw(port_address);

// 32-bit input
uint32_t dword_value = inl(port_address);
```

### Port Output

Write data to hardware ports:

```c
// 8-bit output
outb(port_address, byte_value);

// 16-bit output
outw(port_address, word_value);

// 32-bit output
outl(port_address, dword_value);
```

## Common Hardware Interfaces

### VGA Text Mode Controller

The VGA controller uses ports 0x3D4 and 0x3D5 for cursor control:

```c
void set_vga_cursor(uint16_t position) {
    outb(0x3D4, 0x0F);              // Select low byte register
    outb(0x3D5, position & 0xFF);    // Write low byte
    outb(0x3D4, 0x0E);              // Select high byte register
    outb(0x3D5, (position >> 8) & 0xFF); // Write high byte
}

uint16_t get_vga_cursor(void) {
    outb(0x3D4, 0x0F);              // Select low byte register
    uint8_t low = inb(0x3D5);
    outb(0x3D4, 0x0E);              // Select high byte register
    uint8_t high = inb(0x3D5);
    return (high << 8) | low;
}
```

### Serial Port Communication

COM1 serial port uses ports 0x3F8-0x3FF:

```c
#define COM1_PORT 0x3F8

void serial_init(void) {
    outb(COM1_PORT + 1, 0x00);    // Disable interrupts
    outb(COM1_PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(COM1_PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    outb(COM1_PORT + 1, 0x00);    //                  (hi byte)
    outb(COM1_PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(COM1_PORT + 2, 0xC7);    // Enable FIFO, clear them, 14-byte threshold
    outb(COM1_PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

bool serial_can_write(void) {
    return inb(COM1_PORT + 5) & 0x20;
}

void serial_write_char(char c) {
    while (!serial_can_write());
    outb(COM1_PORT, c);
}
```

### Keyboard Controller

Keyboard controller uses ports 0x60 and 0x64:

```c
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

bool keyboard_has_data(void) {
    return inb(KEYBOARD_STATUS_PORT) & 0x01;
}

uint8_t keyboard_read_scancode(void) {
    while (!keyboard_has_data());
    return inb(KEYBOARD_DATA_PORT);
}

void keyboard_set_leds(bool caps, bool num, bool scroll) {
    uint8_t leds = (caps << 2) | (num << 1) | scroll;
    outb(KEYBOARD_DATA_PORT, 0xED);  // LED command
    while (inb(KEYBOARD_STATUS_PORT) & 0x02); // Wait for input buffer empty
    outb(KEYBOARD_DATA_PORT, leds);
}
```

## Programmable Interval Timer (PIT)

The PIT uses ports 0x40-0x43 for timing operations:

```c
#define PIT_COMMAND_PORT 0x43
#define PIT_DATA_PORT_0  0x40

void pit_init(uint32_t frequency) {
    uint32_t divisor = 1193180 / frequency;

    outb(PIT_COMMAND_PORT, 0x36);  // Channel 0, lobyte/hibyte, mode 3
    outb(PIT_DATA_PORT_0, divisor & 0xFF);        // Low byte
    outb(PIT_DATA_PORT_0, (divisor >> 8) & 0xFF);   // High byte
}
```

## Real-Time Clock (RTC)

RTC uses ports 0x70 and 0x71 for timekeeping:

```c
#define RTC_INDEX_PORT  0x70
#define RTC_DATA_PORT   0x71

uint8_t rtc_read_register(uint8_t reg) {
    outb(RTC_INDEX_PORT, reg);
    return inb(RTC_DATA_PORT);
}

void rtc_write_register(uint8_t reg, uint8_t value) {
    outb(RTC_INDEX_PORT, reg);
    outb(RTC_DATA_PORT, value);
}

uint8_t rtc_get_seconds(void) {
    return rtc_read_register(0x00);
}

uint8_t rtc_get_minutes(void) {
    return rtc_read_register(0x02);
}

uint8_t rtc_get_hours(void) {
    return rtc_read_register(0x04);
}
```

## Best Practices

1. **Check status registers** - Always verify device state before operations
2. **Handle timing** - Some devices require delays between operations
3. **Use proper data widths** - Match port access size to device expectations
4. **Preserve register state** - Save and restore critical register values
5. **Error handling** - Implement timeouts and error checking

## Integration Example

```c
#include "Valen/io.h"

void init_hardware(void) {
    // Initialize serial port for debugging
    serial_init();

    // Set up VGA cursor
    set_vga_cursor(0);

    // Configure system timer
    pit_init(1000);  // 1kHz timer

    // Initialize keyboard
    keyboard_set_leds(false, true, false);  // Num lock on
}

void debug_output(const char* message) {
    for (int i = 0; message[i]; i++) {
        serial_write_char(message[i]);
    }
}
```

## Safety Considerations

- **Port protection** - Some ports may be privileged; ensure proper I/O privilege levels
- **Atomic operations** - Some port sequences must not be interrupted
- **Hardware conflicts** - Avoid accessing the same port from multiple contexts without synchronization
- **Device-specific timing** - Follow manufacturer timing specifications for each device

## Common Port Addresses

| Device         | Port Range  | Description                       |
| -------------- | ----------- | --------------------------------- |
| PIC Master     | 0x20-0x21   | Programmable Interrupt Controller |
| PIC Slave      | 0xA0-0xA1   | Secondary PIC                     |
| Timer (PIT)    | 0x40-0x43   | Programmable Interval Timer       |
| Keyboard       | 0x60-0x64   | Keyboard Controller               |
| COM1           | 0x3F8-0x3FF | Serial Port 1                     |
| COM2           | 0x2F8-0x2FF | Serial Port 2                     |
| VGA            | 0x3C0-0x3DF | Video Graphics Array              |
| RTC            | 0x70-0x71   | Real-Time Clock                   |
| DMA Controller | 0x00-0x0F   | Direct Memory Access              |
