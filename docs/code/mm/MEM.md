# Memory Management

The Memory Management system provides physical memory allocation, virtual memory management, and kernel heap allocation for Valen.

## Overview

Valen uses a four-tier memory management system: Physical Memory Manager (PMM) for tracking physical pages, Paging system for hardware page table management, Virtual Memory Manager (VMM) for address translation, and a kernel heap for dynamic allocation.

## Quick Start

```c
#include <valen/pmm.h>
#include <valen/vmm.h>
#include <valen/heap.h>
#include <valen/paging.h>

void kernel_main(void) {
    // Initialize memory management
    pmm_init(bitmap_addr, memory_size);
    paging_init();
    vmm_init();
    heap_init();

    // Allocate physical page
    void *phys_page = pmm_alloc_page();

    // Allocate virtual memory
    void *virt_mem = vmm_alloc(1, PAGE_PRESENT | PAGE_WRITE);

    // Allocate from kernel heap
    void *heap_mem = malloc(1024);

    // Use memory...

    // Free when done
    pmm_free_page(phys_page);
    free(heap_mem);
}
```

## Physical Memory Manager (PMM)

### Overview

The PMM tracks available physical pages using a bitmap system. Each bit represents one 4KB page.

### Initialization

```c
void pmm_init(uintptr_t start, uint64_t size);
```

**Parameters:**

- `start` - Virtual address where bitmap should be placed
- `size` - Total size of physical RAM in bytes

**Example:**

```c
uintptr_t bitmap_phys = (kernel_phys_end + 0x1000) & ~0xFFFULL;
pmm_init((uintptr_t)PHYS_TO_VIRT(bitmap_phys), max_physical_addr);
```

### Page Allocation

```c
void *pmm_alloc_page(void);
void pmm_free_page(void *addr);
void pmm_mark_free(uintptr_t addr);
void pmm_mark_used(uintptr_t addr);
```

**Example:**

```c
// Allocate a physical page
void *page = pmm_alloc_page();
if (page) {
    // Use the physical page
    printf("Allocated page at: 0x%p\n", page);

    // Free when done
    pmm_free_page(page);
}
```

### Memory Statistics

```c
uint64_t pmm_get_total_kb(void);
uint64_t pmm_get_used_kb(void);
uint64_t pmm_get_free_kb(void);
```

**Example:**

```c
printf("Memory: %d KB total, %d KB used, %d KB free\n",
       pmm_get_total_kb(), pmm_get_used_kb(), pmm_get_free_kb());
```

### PMM Implementation Details

The PMM uses a bitmap where each bit represents a 4KB page:

```c
static uint8_t *bitmap;
static uint64_t bitmap_size;
static uint64_t total_pages;
static uint64_t used_pages;
static spinlock_t pmm_lock = SPINLOCK_INIT;
```

**Safety Features:**

- Never allocates pages below 2MB (reserved for kernel/BIOS)
- Thread-safe with spinlock protection
- All addresses are physical addresses

## Paging System

### Overview

The paging system provides low-level hardware page table management for x86_64. It handles the 4-level paging hierarchy (PML4, PDPT, PD, PT) and manages the translation from virtual to physical addresses.

### Page Flags

```c
#define PAGE_PRESENT (1ULL << 0)  // Page is present
#define PAGE_WRITE   (1ULL << 1)  // Page is writable
#define PAGE_USER    (1ULL << 2)  // Accessible from user mode
#define PAGE_PWT     (1ULL << 3)  // Page Write-Through
#define PAGE_PCD     (1ULL << 4)  // Page-level Cache Disable
#define PAGE_HUGE    (1ULL << 7)  // 2MB/1GB pages
```

### Paging Operations

```c
void paging_init(void);
void paging_map(uint64_t virt, uint64_t phys, uint64_t flags);
void paging_map_range(uint64_t virt, uint64_t phys, uint64_t size, uint64_t flags);
void paging_map_page(uint64_t virt, uint64_t phys, uint64_t flags);
```

### Initialization

```c
void paging_init(void);
```

Initializes the paging system by loading the PML4 table into CR3. This must be called after the page tables are set up by the bootloader.

**Example:**

```c
// Called during kernel initialization
paging_init();
```

### Page Mapping

```c
void paging_map(uint64_t virt, uint64_t phys, uint64_t flags);
```

Maps a single 4KB virtual page to a physical address. Automatically creates the necessary page table hierarchy.

**Parameters:**

- `virt` - Virtual address to map
- `phys` - Physical address to map to
- `flags` - Page protection flags

**Example:**

```c
// Map a virtual page to physical memory
paging_map(0x1000000, physical_addr, PAGE_PRESENT | PAGE_WRITE);
```

### Range Mapping

```c
void paging_map_range(uint64_t virt, uint64_t phys, uint64_t size, uint64_t flags);
```

Maps a contiguous range of memory by iterating through pages.

**Example:**

```c
// Map 64KB of memory
paging_map_range(0x2000000, physical_addr, 0x10000, PAGE_PRESENT | PAGE_WRITE);
```

### Paging Implementation Details

The paging system uses the standard x86_64 4-level page table hierarchy:

```c
// Page table indices for address translation
uint64_t pml4_idx = (virt >> 39) & 0x1FF;  // Level 4
uint64_t pdpt_idx = (virt >> 30) & 0x1FF;  // Page Directory Pointer Table
uint64_t pd_idx  = (virt >> 21) & 0x1FF;  // Page Directory
uint64_t pt_idx  = (virt >> 12) & 0x1FF;  // Page Table
```

**Features:**

- Automatic page table allocation
- Thread-safe with spinlock protection
- Automatic TLB invalidation
- Higher-half virtual address support

**Virtual Address Offset:**

```c
#define KERNEL_VIRT_OFFSET 0xFFFFFFFF80000000
#define PHYS_TO_VIRT(phys) ((void *)((uint64_t)(phys) + KERNEL_VIRT_OFFSET))
```

### Page Table Hierarchy

The paging system automatically creates the 4-level page table structure:

1. **PML4 (Page Map Level 4)** - Top level table
2. **PDPT (Page Directory Pointer Table)** - Second level
3. **PD (Page Directory)** - Third level
4. **PT (Page Table)** - Fourth level, maps to physical pages

Each level is created on-demand when mapping pages.

## Virtual Memory Manager (VMM)

### Overview

The VMM provides virtual-to-physical address translation and manages page tables.

### Page Flags

```c
#define PAGE_PRESENT (1ULL << 0)  // Page is present
#define PAGE_WRITE   (1ULL << 1)  // Page is writable
#define PAGE_USER    (1ULL << 2)  // Accessible from user mode
#define PAGE_PWT     (1ULL << 3)  // Page-level write-through
#define PAGE_PCD     (1ULL << 4)  // Page-level cache disable
#define PAGE_HUGE    (1ULL << 7)  // 2MB/1GB pages
```

### Memory Mapping

```c
void vmm_map(uintptr_t virt, uintptr_t phys, uint64_t flags);
void vmm_map_range(uintptr_t virt, uintptr_t phys, uint64_t size, uint64_t flags);
```

**Example:**

```c
// Map a single page
vmm_map(0x1000000, physical_addr, PAGE_PRESENT | PAGE_WRITE);

// Map a range of memory
vmm_map_range(0x2000000, physical_addr, 0x10000, PAGE_PRESENT | PAGE_WRITE);
```

### Virtual Allocation

```c
void *vmm_alloc(uint64_t pages, uint64_t flags);
```

**Parameters:**

- `pages` - Number of pages to allocate
- `flags` - Page protection flags

**Returns:** Virtual address of allocated memory

**Example:**

```c
// Allocate 4 pages (16KB)
void *memory = vmm_alloc(4, PAGE_PRESENT | PAGE_WRITE);
if (memory) {
    printf("Allocated virtual memory at: 0x%p\n", memory);
}
```

### Address Translation

```c
uintptr_t vmm_get_phys(uintptr_t virtual_addr);
```

**Example:**

```c
uintptr_t phys = vmm_get_phys(virtual_addr);
if (phys) {
    printf("Virtual 0x%p maps to physical 0x%p\n", virtual_addr, phys);
}
```

### VMM Implementation Details

The VMM uses standard x86_64 page table structures:

```c
// Page table indices for address translation
uint64_t pml4_idx = (virtual_addr >> 39) & 0x1FF;
uint64_t pdpt_idx = (virtual_addr >> 30) & 0x1FF;
uint64_t pd_idx  = (virtual_addr >> 21) & 0x1FF;
uint64_t pt_idx  = (virtual_addr >> 12) & 0x1FF;
```

**Features:**

- Supports 4KB, 2MB, and 1GB pages
- Automatic TLB invalidation
- Thread-safe with spinlock protection

## Kernel Heap

### Overview

The kernel heap provides dynamic memory allocation using a linked-list based allocator.

### Heap Operations

```c
void heap_init(void);
void *malloc(uint64_t size);
void free(void *ptr);
```

### Allocation Example

```c
void heap_example(void) {
    // Allocate memory
    char *buffer = malloc(256);
    if (buffer) {
        // Use the memory
        strcpy(buffer, "Hello from heap!");
        printf("Heap buffer: %s\n", buffer);

        // Free when done
        free(buffer);
    }
}
```

### Heap Implementation Details

The heap uses a node-based allocator:

```c
typedef struct heap_node {
    uint32_t magic;        // Magic number for validation
    uint64_t size;         // Size of this block
    struct heap_node *next; // Next node in list
    int free;              // 1 if free, 0 if allocated
} heap_node_t;

#define HEAP_MAGIC 0x12345678
```

**Features:**

- Automatic coalescing of adjacent free blocks
- 8-byte alignment for all allocations
- Magic number validation for corruption detection
- Thread-safe with spinlock protection

## Memory Layout

### Virtual Address Space

```
0xFFFFFFFF80000000 - 0xFFFFFFFFB0000000 : Kernel code and data
0xFFFFFFFFB0000000 - 0xFFFFFFFFC0000000 : Kernel heap allocations
0xFFFFFFFFC0000000 - 0xFFFFFFFFFEFFFFFF : Device mappings
```

### Physical Memory Protection

The PMM protects critical memory regions:

- **First 2MB (0x0 - 0x200000)** - Reserved for hardware/BIOS
- **Kernel memory** - Protected during allocation
- **PMM bitmap** - Protected to prevent corruption

## Usage Patterns

### Basic Memory Allocation

```c
void basic_allocation(void) {
    // Allocate from kernel heap
    void *heap_mem = malloc(1024);

    // Allocate virtual memory
    void *virt_mem = vmm_alloc(2, PAGE_PRESENT | PAGE_WRITE);

    // Allocate physical page directly
    void *phys_page = pmm_alloc_page();

    // Use memory...

    // Clean up
    free(heap_mem);
    // Note: vmm_alloc and pmm_alloc_page don't have free equivalents
    // in the current implementation
}
```

### Device Memory Mapping

```c
void map_device_registers(uintptr_t phys_addr, size_t size) {
    // Map device registers with cache disabled
    uint64_t flags = PAGE_PRESENT | PAGE_WRITE | PAGE_PCD;
    vmm_map_range(0xF0000000, phys_addr, size, flags);

    // Now access device registers at 0xF0000000
    volatile uint32_t *regs = (volatile uint32_t *)0xF0000000;
    regs[0] = 0xDEADBEEF;
}
```

### Memory Statistics

```c
void print_memory_info(void) {
    printf("Physical Memory:\n");
    printf("  Total: %d KB\n", pmm_get_total_kb());
    printf("  Used:  %d KB\n", pmm_get_used_kb());
    printf("  Free:  %d KB\n", pmm_get_free_kb());

    // Calculate usage percentage
    uint64_t total = pmm_get_total_kb();
    uint64_t used = pmm_get_used_kb();
    if (total > 0) {
        printf("  Usage: %d%%\n", (used * 100) / total);
    }
}
```

## Best Practices

1. **Check return values** - All allocation functions can fail
2. **Free heap memory** - Prevent memory leaks
3. **Use appropriate flags** - Set correct page protection
4. **Avoid fragmentation** - Use appropriate allocation sizes
5. **Protect device memory** - Use PAGE_PCD for MMIO regions

## Error Handling

```c
void robust_memory_allocation(void) {
    void *buffer = malloc(1024);
    if (!buffer) {
        printf("Heap allocation failed\n");
        return;
    }

    void *pages = vmm_alloc(4, PAGE_PRESENT | PAGE_WRITE);
    if (!pages) {
        printf("Virtual allocation failed\n");
        free(buffer);
        return;
    }

    void *phys = pmm_alloc_page();
    if (!phys) {
        printf("Physical allocation failed\n");
        free(buffer);
        return;
    }

    // Use memory...

    free(buffer);
}
```

## Integration Example

```c
#include <valen/pmm.h>
#include <valen/vmm.h>
#include <valen/heap.h>
#include <valen/paging.h>

void init_memory_subsystem(void) {
    // Initialize PMM with memory map from bootloader
    extern char _kernel_end[];
    uintptr_t kernel_phys_end = VIRT_TO_PHYS((uintptr_t)_kernel_end);
    uintptr_t bitmap_phys = (kernel_phys_end + 0x1000) & ~0xFFFULL;

    pmm_init((uintptr_t)PHYS_TO_VIRT(bitmap_phys), max_physical_addr);

    // Initialize paging system
    paging_init();

    // Initialize VMM
    vmm_init();

    // Initialize kernel heap
    heap_init();
}

void allocate_resources(void) {
    // Allocate heap memory for data structures
    struct my_data *data = malloc(sizeof(struct my_data));
    if (!data) return;

    // Allocate virtual memory for buffers
    void *buffer_area = vmm_alloc(8, PAGE_PRESENT | PAGE_WRITE);
    if (!buffer_area) {
        free(data);
        return;
    }

    // Use resources...

    // Clean up
    free(data);
}
```

This memory management system provides a robust foundation for kernel memory operations with proper protection, allocation tracking, and efficient page management.
