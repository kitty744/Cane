# Boot Process

The CaneOS boot process handles system initialization from bootloader handoff to kernel startup.

## Overview

CaneOS uses the Multiboot2 specification for bootloader compatibility. The boot process initializes hardware, sets up memory management, establishes interrupt handling, and transitions the system from real mode to protected mode and finally to 64-bit long mode.

## Boot Sequence

### 1. Bootloader Handoff

The bootloader (GRUB2) loads the kernel and passes control via Multiboot2 information:

```c
// Entry point in boot.s
[bits 64]
[extern kmain]

global _start
_start:
    ; Save multiboot info pointer
    mov rdi, rdi    ; Multiboot info structure
    mov rsi, rsi    ; Magic number

    ; Call kernel main
    call kmain

    ; Halt if kernel returns
    cli
.halt:
    hlt
    jmp .halt
```

### 2. Early Kernel Initialization

```c
#include "cane/boot.h"

void kmain(uint64_t mb_info, uint64_t mb_magic) {
    // Verify multiboot magic
    if (mb_magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        early_panic("Invalid multiboot magic");
    }

    // Initialize core systems
    boot_init(mb_info);

    // Start the kernel
    kernel_start();
}
```

## Boot Phases

### Phase 1: Hardware Detection

```c
void boot_detect_hardware(struct multiboot_info *mb_info) {
    // Parse memory map
    parse_memory_map(mb_info);

    // Detect CPU features
    detect_cpu_features();

    // Initialize basic hardware
    init_basic_hardware();

    // Set up console
    console_init();
}
```

### Phase 2: Memory Setup

```c
void boot_setup_memory(struct multiboot_info *mb_info) {
    // Initialize physical memory manager
    pmm_init_from_multiboot(mb_info);

    // Set up virtual memory
    vmm_init_early();

    // Initialize kernel heap
    kheap_init();

    // Map kernel space
    map_kernel_space();
}
```

### Phase 3: Interrupt System

```c
void boot_setup_interrupts(void) {
    // Initialize interrupt descriptor table
    idt_init();

    // Set up interrupt handlers
    setup_interrupt_handlers();

    // Enable interrupts
    enable_interrupts();

    // Initialize timer
    timer_init();
}
```

### Phase 4: Device Initialization

```c
void boot_init_devices(void) {
    // Initialize VGA display
    vga_init();

    // Set up serial port
    serial_init();

    // Initialize keyboard
    keyboard_init();

    // Detect and initialize storage devices
    storage_init();
}
```

## Memory Map Parsing

### Multiboot Memory Map

```c
struct memory_region {
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
    uint32_t reserved;
};

void parse_memory_map(struct multiboot_info *mb_info) {
    struct multiboot_tag *tag;

    for (tag = (struct multiboot_tag *)(mb_info + 8);
         tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = (struct multiboot_tag *)((uint8_t *)tag +
              ((tag->size + 7) & ~7))) {

        switch (tag->type) {
        case MULTIBOOT_TAG_TYPE_MMAP:
            parse_mmap_tag((struct multiboot_tag_mmap *)tag);
            break;
        case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
            parse_basic_meminfo((struct multiboot_tag_basic_meminfo *)tag);
            break;
        }
    }
}

void parse_mmap_tag(struct multiboot_tag_mmap *mmap_tag) {
    struct multiboot_mmap_entry *entry;

    for (entry = mmap_tag->entries;
         (uint8_t *)entry < (uint8_t *)mmap_tag + mmap_tag->size;
         entry = (struct multiboot_mmap_entry *)
                ((uint8_t *)entry + mmap_tag->entry_size)) {

        if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
            // Add available memory to physical memory manager
            pmm_add_region(entry->addr, entry->len);
        } else {
            // Mark reserved regions
            pmm_reserve_region(entry->addr, entry->len);
        }
    }
}
```

## CPU Feature Detection

```c
struct cpu_features {
    bool has_apic;
    bool has_msr;
    bool has_pat;
    bool has_nx;
    bool has_sse;
    bool has_sse2;
    bool has_avx;
    uint8_t cpu_family;
    uint8_t cpu_model;
    char vendor[13];
};

void detect_cpu_features(void) {
    struct cpu_features *cpu = &g_cpu_features;

    // Get CPU vendor
    uint32_t eax, ebx, ecx, edx;
    cpuid(0, &eax, &ebx, &ecx, &edx);

    // Store vendor string
    *((uint32_t*)&cpu->vendor[0]) = ebx;
    *((uint32_t*)&cpu->vendor[4]) = edx;
    *((uint32_t*)&cpu->vendor[8]) = ecx;
    cpu->vendor[12] = '\0';

    // Get feature flags
    cpuid(1, &eax, &ebx, &ecx, &edx);

    cpu->has_sse   = (edx & (1 << 25)) != 0;
    cpu->has_sse2  = (edx & (1 << 26)) != 0;
    cpu->has_apic  = (edx & (1 << 9))  != 0;
    cpu->has_msr   = (edx & (1 << 5))  != 0;
    cpu->has_nx    = (edx & (1 << 20)) != 0;

    // Get extended features
    cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
    cpu->has_nx = cpu->has_nx || ((edx & (1 << 20)) != 0);

    // Get family and model
    cpu->cpu_family = ((eax >> 8) & 0xF) + ((eax >> 20) & 0xFF);
    cpu->cpu_model  = ((eax >> 4) & 0xF) + ((eax >> 12) & 0xF0);

    printf("CPU: %s Family %d Model %d\n",
           cpu->vendor, cpu->cpu_family, cpu->cpu_model);
}
```

## Early Console Setup

```c
void console_init(void) {
    // Initialize VGA text mode
    vga_init();

    // Set up early printf
    early_printf_init();

    // Clear screen and show boot message
    vga_clear();
    vga_set_color(0x0F);  // White on black
    vga_printf("CaneOS v%s Booting...\n", KERNEL_VERSION);

    // Initialize serial for debugging
    serial_init();
    serial_printf("CaneOS boot started\n");
}
```

## Kernel Image Loading

### ELF Section Parsing

```c
struct kernel_section {
    uint64_t virt_addr;
    uint64_t phys_addr;
    uint64_t size;
    uint32_t flags;
    const char *name;
};

void load_kernel_sections(void) {
    extern uint8_t _start, _end;

    // Calculate kernel size
    uint64_t kernel_size = (uint64_t)&_end - (uint64_t)&_start;

    // Map kernel sections with appropriate permissions
    map_kernel_text(&_start, &_etext, PAGE_PRESENT | PAGE_EXECUTIVE);
    map_kernel_data(&_etext, &_edata, PAGE_PRESENT | PAGE_WRITE);
    map_kernel_bss(&_edata, &_end, PAGE_PRESENT | PAGE_WRITE);

    printf("Kernel loaded: %d KB\n", kernel_size / 1024);
}

void map_kernel_text(uint8_t *start, uint8_t *end, uint32_t flags) {
    for (uint64_t addr = (uint64_t)start; addr < (uint64_t)end; addr += 0x1000) {
        vmm_map_page(addr, addr - KERNEL_VIRTUAL_BASE, flags);
    }
}
```

## Interrupt Controller Setup

```c
void setup_interrupt_controller(void) {
    // Initialize APIC if available
    if (g_cpu_features.has_apic) {
        init_apic();
    } else {
        // Fall back to legacy PIC
        init_pic();
    }

    // Set up interrupt handlers
    setup_exception_handlers();
    setup_irq_handlers();

    // Enable interrupts
    sti();
}

void init_pic(void) {
    // Remap PIC interrupts
    outb(0x20, 0x11);  // ICW1: Initialize
    outb(0x21, 0x20);  // ICW2: Vector offset
    outb(0xA1, 0x28);  // ICW2: Slave vector offset
    outb(0x21, 0x04);  // ICW3: Master/slave
    outb(0xA1, 0x02);  // ICW3: Master/slave
    outb(0x21, 0x01);  // ICW4: Mode
    outb(0xA1, 0x01);  // ICW4: Mode

    // Mask all interrupts initially
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);
}
```

## Boot Configuration

### Kernel Command Line

```c
void parse_kernel_command_line(struct multiboot_info *mb_info) {
    struct multiboot_tag_string *cmdline_tag;

    cmdline_tag = find_multiboot_tag(mb_info, MULTIBOOT_TAG_TYPE_CMDLINE);
    if (cmdline_tag) {
        parse_boot_options(cmdline_tag->string);
    }
}

void parse_boot_options(const char *cmdline) {
    char *cmdline_copy = strdup(cmdline);
    char *token = strtok(cmdline_copy, " ");

    while (token != NULL) {
        if (strcmp(token, "debug") == 0) {
            g_debug_enabled = true;
        } else if (strcmp(token, "quiet") == 0) {
            g_quiet_boot = true;
        } else if (strncmp(token, "init=", 5) == 0) {
            g_init_program = token + 5;
        }

        token = strtok(NULL, " ");
    }

    free(cmdline_copy);
}
```

## Boot Time Optimization

### Parallel Initialization

```c
void boot_parallel_init(void) {
    // Start secondary CPUs if available
    if (g_cpu_features.has_apic) {
        start_secondary_cpus();
    }

    // Parallel device initialization
    parallel_init_devices();
}

void parallel_init_devices(void) {
    // Initialize devices in parallel where possible
    start_thread(init_storage_devices);
    start_thread(init_network_devices);
    start_thread(init_input_devices);

    // Wait for all devices to initialize
    wait_for_device_init();
}
```

### Fast Boot Paths

```c
void fast_boot_path(void) {
    // Skip time-consuming operations in fast boot mode
    if (g_fast_boot) {
        // Skip memory tests
        skip_memory_tests();

        // Use cached device detection
        use_cached_device_info();

        // Initialize only essential devices
        init_essential_devices_only();
    }
}
```

## Error Handling

### Boot Failure Recovery

```c
void boot_panic(const char *message) {
    // Disable interrupts
    cli();

    // Show error on screen
    vga_set_color(0x04);  // Red on black
    vga_clear();
    vga_printf("BOOT PANIC: %s\n", message);

    // Output to serial
    serial_printf("BOOT PANIC: %s\n", message);

    // Halt system
    while (1) {
        hlt();
    }
}

void boot_warning(const char *message) {
    vga_set_color(0x0E);  // Yellow on black
    vga_printf("BOOT WARNING: %s\n", message);
    serial_printf("BOOT WARNING: %s\n", message);
}
```

## Boot Statistics

```c
struct boot_stats {
    uint64_t start_time;
    uint64_t end_time;
    uint64_t memory_detected;
    uint32_t devices_found;
    uint32_t modules_loaded;
};

void record_boot_stats(void) {
    g_boot_stats.start_time = get_boot_time();

    // Record various boot metrics
    g_boot_stats.memory_detected = get_total_memory();
    g_boot_stats.devices_found = count_initialized_devices();
    g_boot_stats.modules_loaded = count_loaded_modules();

    g_boot_stats.end_time = get_boot_time();
}

void print_boot_summary(void) {
    uint64_t boot_time = g_boot_stats.end_time - g_boot_stats.start_time;

    printf("Boot completed in %d ms\n", boot_time);
    printf("Memory: %d MB\n", g_boot_stats.memory_detected / (1024 * 1024));
    printf("Devices: %d\n", g_boot_stats.devices_found);
    printf("Modules: %d\n", g_boot_stats.modules_loaded);
}
```

## Integration Example

```c
void kmain(uint64_t mb_info, uint64_t mb_magic) {
    // Early initialization
    early_init(mb_info, mb_magic);

    // Main boot sequence
    boot_sequence();

    // Start kernel
    kernel_start();

    // Should never reach here
    boot_panic("Kernel returned unexpectedly");
}

void boot_sequence(void) {
    // Phase 1: Hardware detection
    detect_hardware();

    // Phase 2: Memory setup
    setup_memory();

    // Phase 3: Interrupt system
    setup_interrupts();

    // Phase 4: Device initialization
    init_devices();

    // Phase 5: Start secondary CPUs
    start_secondary_cpus();

    // Phase 6: Initialize subsystems
    init_subsystems();

    // Record boot completion
    record_boot_stats();
    print_boot_summary();
}
```

This boot process provides a robust foundation for system initialization with proper error handling, optimization, and extensibility.
