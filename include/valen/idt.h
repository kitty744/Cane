#ifndef IDT_H
#define IDT_H

#include <stdint.h>

// A 64-bit IDT entry is 16 bytes
struct idt_entry
{
    uint16_t isr_low;   // Lower 16 bits of ISR address
    uint16_t kernel_cs; // Kernel code segment (from GDT)
    uint8_t ist;        // Interrupt Stack Table
    uint8_t attributes; // Type and attributes (e.g., 0x8E for interrupt gate)
    uint16_t isr_mid;   // Middle 16 bits of ISR address
    uint32_t isr_high;  // Upper 32 bits of ISR address
    uint32_t reserved;  // Set to 0
} __attribute__((packed));

struct idt_ptr
{
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

void idt_init();
void idt_set_descriptor(uint8_t vector, void *isr, uint8_t flags);

#endif