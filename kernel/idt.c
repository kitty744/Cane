/**
 * @file idt.c
 * @brief Interrupt Descriptor Table (IDT) Management for NexusOS.
 *
 * This module initializes the IDT, which is the mechanism the x86_64 CPU uses
 * to handle exceptions and hardware interrupts. It maps assembly-level
 * Interrupt Service Routine (ISR) stubs to their respective vectors.
 */

#include <cane/idt.h>

/* --- Global IDT Structures --- */

/** @brief The actual table of 256 interrupt descriptors. */
struct idt_entry idt[256];

/** @brief The IDTR pointer structure passed to the 'lidt' instruction. */
struct idt_ptr idtp;

/* --- External Assembly Stubs --- */

extern void page_fault_isr();
extern void load_idt(struct idt_ptr *ptr);

/**
 * @brief Configures an individual IDT gate.
 * * @param vector The interrupt vector index (0-255).
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
 * * This function performs the following steps:
 * 1. Disables the legacy 8259 PIC to favor the modern APIC.
 * 2. Populates all 256 entries with a default safety handler.
 * 3. Registers specific CPU exceptions (e.g., Page Faults).
 * 4. Registers hardware IRQ stubs (Timer, Keyboard, Mouse).
 * 5. Loads the IDT pointer into the CPU's IDTR register.
 */
void idt_init()
{

    /* 3. Register CPU Exceptions (Vectors 0-31) */
    /* Vector 14: Page Fault - Critical for Virtual Memory Management */
    idt_set_descriptor(14, page_fault_isr, 0x8E);

    /* 6. Configure IDT Pointer and load into CPU register */
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (uint64_t)&idt;

    load_idt(&idtp);
}