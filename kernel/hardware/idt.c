/**
 * @file idt.c
 * @brief Interrupt Descriptor Table (IDT) Management for Valen.
 *
 * This module initializes the IDT, which is the mechanism the x86_64 CPU uses
 * to handle exceptions and hardware interrupts. It maps assembly-level
 * Interrupt Service Routine (ISR) stubs to their respective vectors.
 */

#include <valen/idt.h>
#include <valen/pic.h>
#include <valen/keyboard.h>

/* --- Global IDT Structures --- */

/** @brief The actual table of 256 interrupt descriptors. */
struct idt_entry idt[256];

/** @brief The IDTR pointer structure passed to the 'lidt' instruction. */
struct idt_ptr idtp;

/* --- External Assembly Stubs --- */

extern void page_fault_isr();
extern void keyboard_isr();
extern void generic_isr();
extern void scheduler_tick();
extern void timer_isr();
extern void load_idt(struct idt_ptr *ptr);

/* --- Generic Handler --- */

void generic_handler(void)
{
    /* Generic interrupt handler - just send EOI and return */
    pic_send_eoi(0);
}

/**
 * @brief Configures an individual IDT gate.
 * @param vector The interrupt vector index (0-255).
 * @param isr    Pointer to the assembly ISR stub.
 * @param flags  Descriptor attributes (typically 0x8E for a 64-bit Interrupt Gate).
 */
void idt_set_descriptor(uint8_t vector, void *isr, uint8_t flags)
{
    uint64_t addr = (uint64_t)isr;

    idt[vector].isr_low = addr & 0xFFFF;
    idt[vector].kernel_cs = 0x08; /* Kernel Code Segment Offset */
    idt[vector].ist = 0;          /* Interrupt Stack Table (not used) */
    idt[vector].attributes = flags;
    idt[vector].isr_mid = (addr >> 16) & 0xFFFF;
    idt[vector].isr_high = (addr >> 32) & 0xFFFFFFFF;
    idt[vector].reserved = 0;
}

/**
 * @brief Initializes the IDT and prepares the CPU for interrupt handling.
 * This function performs the following steps:
 * 1. Initialize PIC and remap interrupts
 * 2. Initialize all vectors with a default generic handler
 * 3. Register specific CPU exceptions (e.g., Page Faults)
 * 4. Register hardware IRQ stubs (Timer, Keyboard, Mouse)
 * 5. Load the IDT pointer into the CPU's IDTR register
 */
void idt_init()
{
    /* 1. Initialize PIC and remap interrupts */
    pic_init();

    /* 2. Initialize all vectors with a default generic handler to prevent triple faults */
    for (int i = 0; i < 256; i++)
    {
        idt_set_descriptor(i, generic_isr, 0x8E);
    }

    /* 3. Register CPU Exceptions (Vectors 0-31) */
    /* Vector 14: Page Fault - Critical for Virtual Memory Management */
    idt_set_descriptor(14, page_fault_isr, 0x8E);

    /* 4. Register Hardware IRQs */
    /* IRQ 1: Keyboard - Vector 0x21 (0x20 + 1) */
    idt_set_descriptor(33, keyboard_isr, 0x8E);
    
    /* IRQ 0: Timer - Vector 0x20 (0x20 + 0) */
    idt_set_descriptor(32, timer_isr, 0x8E);

    /* 5. Configure IDT Pointer and load into CPU register */
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (uint64_t)&idt;

    load_idt(&idtp);
}