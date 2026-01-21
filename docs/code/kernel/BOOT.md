# Boot Process

The Valen boot process handles system initialization from bootloader handoff to kernel startup.

## Overview

Valen uses the Multiboot2 specification for bootloader compatibility. The boot process initializes hardware, sets up memory management, establishes interrupt handling, and starts the shell interface.

## Boot Sequence

### Entry Point

The kernel entry point is the `kmain` function which receives the multiboot magic number and info structure:

```c
#include <valen/kernel.h>

void kmain(unsigned long magic, unsigned long addr)
{
    print_clear();

    idt_init();
    gdt_init();

    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC)
    {
        while (1)
            ;
    }

    // ... rest of initialization
}
```

## Boot Phases

### Phase 1: Basic Hardware Setup

```c
void kmain(unsigned long magic, unsigned long addr)
{
    // Clear screen
    print_clear();

    // Initialize interrupt handling
    idt_init();
    gdt_init();

    // Verify multiboot magic
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC)
    {
        while (1)
            ;
    }
}
```

### Phase 2: Memory Map Parsing

The kernel parses the multiboot memory map to determine available memory:

```c
uint64_t max_physical_addr = 0;
struct multiboot_tag_mmap *mmap_tag = NULL;

struct multiboot_tag *tag = (struct multiboot_tag *)PHYS_TO_VIRT(addr + 8);
while (tag->type != MULTIBOOT_TAG_TYPE_END)
{
    if (tag->type == MULTIBOOT_TAG_TYPE_MMAP)
    {
        mmap_tag = (struct multiboot_tag_mmap *)tag;
        uint32_t entries = (mmap_tag->size - sizeof(struct multiboot_tag_mmap)) / mmap_tag->entry_size;
        for (uint32_t i = 0; i < entries; i++)
        {
            if (mmap_tag->entries[i].type == MULTIBOOT_MEMORY_AVAILABLE)
            {
                uint64_t end = mmap_tag->entries[i].addr + mmap_tag->entries[i].len;
                if (end > max_physical_addr)
                    max_physical_addr = end;
            }
        }
    }
    tag = (struct multiboot_tag *)((uint8_t *)tag + ((tag->size + 7) & ~7));
}

if (max_physical_addr == 0)
    max_physical_addr = 0x20000000;
```

### Phase 3: Physical Memory Manager Initialization

```c
uintptr_t kernel_phys_end = VIRT_TO_PHYS((uintptr_t)_kernel_end);
uintptr_t bitmap_phys = (kernel_phys_end + 0x1000) & ~0xFFFULL;
pmm_init((uintptr_t)PHYS_TO_VIRT(bitmap_phys), max_physical_addr);

if (mmap_tag)
{
    uint32_t entries = (mmap_tag->size - sizeof(struct multiboot_tag_mmap)) / mmap_tag->entry_size;
    uint64_t bitmap_size = (max_physical_addr / 32768) + 4096;
    uintptr_t b_end = bitmap_phys + bitmap_size;

    for (uint32_t i = 0; i < entries; i++)
    {
        if (mmap_tag->entries[i].type == MULTIBOOT_MEMORY_AVAILABLE)
        {
            for (uint64_t a = mmap_tag->entries[i].addr; a < mmap_tag->entries[i].addr + mmap_tag->entries[i].len; a += 4096)
            {
                if (a < 0x200000 || (a >= bitmap_phys && a < b_end))
                    continue;
                pmm_mark_free(a);
            }
        }
    }
}
```

### Phase 4: Virtual Memory and Heap Setup

```c
vmm_init();
heap_init();
```

### Phase 5: Device Initialization

```c
keyboard_init();
```

### Phase 6: User Interface

```c
set_color(COLOR_DARK_GREY);
printf("Type 'help' to begin.\n");
set_color(COLOR_GREEN);
shell_init();

system_ready = 1;

/* Enable CPU interrupts */
asm volatile ("sti");

while (1) {
    asm volatile ("hlt");
}
```

## Memory Layout

### Virtual Address Mapping

The kernel uses a fixed virtual offset for kernel memory:

```c
#define KERNEL_VIRT_OFFSET 0xFFFFFFFF80000000ULL
#define PHYS_TO_VIRT(p) ((void *)((uint64_t)(p) + KERNEL_VIRT_OFFSET))
#define VIRT_TO_PHYS(v) ((uint64_t)(v) - KERNEL_VIRT_OFFSET)
```

### Kernel End Symbol

The kernel uses the `_kernel_end` symbol to determine where kernel memory ends:

```c
extern char _kernel_end[];
```

## Multiboot2 Integration

### Magic Number Verification

```c
#define MULTIBOOT2_BOOTLOADER_MAGIC 0x36d76289

if (magic != MULTIBOOT2_BOOTLOADER_MAGIC)
{
    while (1)
        ;
}
```

### Memory Types

The kernel recognizes these memory types from the multiboot specification:

```c
#define MULTIBOOT_MEMORY_AVAILABLE 1
#define MULTIBOOT_MEMORY_RESERVED 2
#define MULTIBOOT_MEMORY_ACPI_RECLAIM 3
#define MULTIBOOT_MEMORY_NVS 4
#define MULTIBOOT_MEMORY_BADRAM 5
```

## System State

### Ready Flag

The kernel uses a global flag to indicate when the system is ready:

```c
int system_ready = 0;

// Later in initialization:
system_ready = 1;
```

### Interrupt Enable

The kernel enables interrupts after all initialization is complete:

```c
/* Enable CPU interrupts */
asm volatile ("sti");
```

## Boot Configuration

### Default Memory Size

If no memory map is available, the kernel defaults to 512MB:

```c
if (max_physical_addr == 0)
    max_physical_addr = 0x20000000;  // 512MB
```

### Memory Protection

The kernel protects certain memory regions:

- First 2MB (0x0 - 0x200000) - Reserved for hardware
- PMM bitmap region - Protected to prevent corruption

## Error Handling

### Boot Failure

If the multiboot magic is invalid, the kernel halts:

```c
if (magic != MULTIBOOT2_BOOTLOADER_MAGIC)
{
    while (1)
        ;
}
```

## Integration Example

```c
#include <valen/kernel.h>
#include <valen/stdio.h>
#include <valen/color.h>
#include <valen/multiboot.h>
#include <valen/idt.h>
#include <valen/gdt.h>
#include <valen/pmm.h>
#include <valen/vmm.h>
#include <valen/heap.h>
#include <valen/shell.h>
#include <valen/keyboard.h>

void kmain(unsigned long magic, unsigned long addr)
{
    // Phase 1: Basic setup
    print_clear();
    idt_init();
    gdt_init();

    // Phase 2: Verify bootloader
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        while (1);
    }

    // Phase 3: Parse memory map
    // Phase 4: Initialize memory managers
    // Phase 5: Initialize devices
    // Phase 6: Start user interface

    // Enable interrupts and halt
    asm volatile ("sti");
    while (1) {
        asm volatile ("hlt");
    }
}
```

This boot process provides a straightforward initialization sequence that sets up the essential kernel components and prepares the system for user interaction.
