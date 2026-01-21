/**
 * @file pit.c
 * @brief Programmable Interval Timer (PIT) Driver for Valen
 * 
 * This module implements the Intel 8253/8254 Programmable Interval Timer (PIT)
 * which provides timer interrupts for task scheduling and system timing.
 */

#include <valen/io.h>
#include <valen/pic.h>

#define PIT_COMMAND_PORT 0x43
#define PIT_DATA_PORT_0 0x40

/**
 * @brief Initialize the PIT with specified frequency
 * @param frequency The desired timer frequency in Hz
 */
void pit_init(uint32_t frequency) {
    uint32_t divisor = 1193180 / frequency;
    
    // Configure Channel 0, square wave mode, access mode, lobyte/hibyte, mode 3
    outb(PIT_COMMAND_PORT, 0x36);
    outb(PIT_DATA_PORT_0, divisor & 0xFF);        // Low byte
    outb(PIT_DATA_PORT_0, (divisor >> 8) & 0xFF);   // High byte
    
    // Enable timer IRQ (IRQ 0) in PIC
    pic_irq_enable(0);
}
