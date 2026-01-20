# Memory Management

The Memory Management system provides dynamic memory allocation, virtual memory management, and memory protection for CaneOS.

## Overview

CaneOS uses a sophisticated memory management system that combines physical memory management, virtual memory mapping, and kernel heap allocation. The system supports both kernel and user space memory management with proper isolation and protection.

## Quick Start

```c
#include "cane/mm.h"

void kernel_main(void) {
    // Initialize memory management
    mm_init();

    // Allocate kernel memory
    void *buffer = kmalloc(1024);
    if (buffer) {
        memset(buffer, 0, 1024);
        // Use buffer...
        kfree(buffer);
    }

    // Allocate page-aligned memory
    void *page_buffer = kmalloc_page(4096);

    // Map virtual memory
    void *virt_addr = mm_map_physical(physical_addr, size, flags);
}
```

## Memory Architecture

### Physical Memory Management

The physical memory manager tracks available physical pages and provides allocation services:

```c
// Initialize physical memory manager
void pmm_init(uint64_t memory_size, struct multiboot_mmap_entry *mmap);

// Allocate/free physical pages
uint64_t pmm_alloc_page(void);
void pmm_free_page(uint64_t page_addr);

// Allocate multiple pages
uint64_t pmm_alloc_pages(size_t count);
void pmm_free_pages(uint64_t page_addr, size_t count);
```

### Virtual Memory Management

Virtual memory provides address space isolation and memory protection:

```c
// Initialize virtual memory manager
void vmm_init(void);

// Map/unmap virtual pages
int vmm_map_page(uint64_t virt_addr, uint64_t phys_addr, uint32_t flags);
int vmm_unmap_page(uint64_t virt_addr);

// Get physical address from virtual
uint64_t vmm_get_physical(uint64_t virt_addr);

// Change page protection
int vmm_protect_page(uint64_t virt_addr, uint32_t flags);
```

### Kernel Heap Management

The kernel heap provides dynamic memory allocation for kernel components:

```c
// Initialize kernel heap
void kheap_init(void);

// Allocate/free kernel memory
void *kmalloc(size_t size);
void *kcalloc(size_t count, size_t size);
void *krealloc(void *ptr, size_t size);
void kfree(void *ptr);

// Allocate page-aligned memory
void *kmalloc_page(size_t size);
```

## Memory Allocation Patterns

### Basic Kernel Allocation

```c
void basic_allocation_example(void) {
    // Simple allocation
    char *buffer = kmalloc(256);
    if (buffer) {
        strcpy(buffer, "Hello, kernel!");
        printf("Buffer: %s\n", buffer);
        kfree(buffer);
    }

    // Zero-initialized allocation
    struct my_struct *obj = kcalloc(1, sizeof(struct my_struct));
    if (obj) {
        // obj->fields are already zeroed
        obj->id = 42;
        kfree(obj);
    }

    // Reallocation
    char *dynamic = kmalloc(64);
    if (dynamic) {
        strcpy(dynamic, "Initial content");

        // Expand buffer
        char *expanded = krealloc(dynamic, 128);
        if (expanded) {
            strcat(expanded, " - expanded!");
            printf("Expanded: %s\n", expanded);
            kfree(expanded);
        }
    }
}
```

### Page-Aligned Allocation

```c
void page_aligned_example(void) {
    // Allocate page-aligned memory for DMA or hardware interfaces
    void *dma_buffer = kmalloc_page(4096);
    if (dma_buffer) {
        // Buffer is guaranteed to be page-aligned
        printf("DMA buffer at: 0x%p\n", dma_buffer);

        // Use for hardware operations
        setup_dma_transfer(dma_buffer, 4096);

        kfree(dma_buffer);
    }

    // Allocate multiple pages
    void *large_buffer = kmalloc_page(64 * 1024);  // 64KB
    if (large_buffer) {
        // Use large buffer...
        kfree(large_buffer);
    }
}
```

### Virtual Memory Mapping

```c
void virtual_memory_example(void) {
    // Map physical memory to virtual address
    uint64_t phys_addr = 0x1000000;  // Some physical address
    size_t size = 4096;
    uint32_t flags = PAGE_PRESENT | PAGE_WRITE;

    void *virt_addr = mm_map_physical(phys_addr, size, flags);
    if (virt_addr) {
        // Access physical memory through virtual address
        uint32_t *memory = (uint32_t*)virt_addr;
        *memory = 0xDEADBEEF;

        // Unmap when done
        mm_unmap_physical(virt_addr, size);
    }
}
```

## Memory Protection and Flags

### Page Protection Flags

```c
// Common page flags
#define PAGE_PRESENT    (1 << 0)  // Page is present
#define PAGE_WRITE      (1 << 1)  // Page is writable
#define PAGE_USER       (1 << 2)  // Page is accessible from user mode
#define PAGE_WRITE_THROUGH (1 << 3)  // Write-through caching
#define PAGE_CACHE_DISABLE (1 << 4)  // Disable caching
#define PAGE_ACCESSED   (1 << 5)  // Page was accessed (set by CPU)
#define PAGE_DIRTY      (1 << 6)  // Page was written to (set by CPU)

// Example: Create user-accessible, writable page
uint32_t user_flags = PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
vmm_map_page(virt_addr, phys_addr, user_flags);

// Example: Create kernel-only, read-only page
uint32_t kernel_ro_flags = PAGE_PRESENT;
vmm_map_page(virt_addr, phys_addr, kernel_ro_flags);
```

### Memory Regions

```c
// Define memory regions with different properties
struct memory_region {
    uint64_t start;
    uint64_t end;
    uint32_t flags;
    const char *name;
};

void setup_memory_regions(void) {
    // Kernel code region (read-only, executable)
    struct memory_region kernel_code = {
        .start = 0xFFFFFFFF80000000,
        .end = 0xFFFFFFFF800FFFFF,
        .flags = PAGE_PRESENT | PAGE_EXECUTIVE,
        .name = "kernel_code"
    };

    // Kernel data region (read-write)
    struct memory_region kernel_data = {
        .start = 0xFFFFFFFF80100000,
        .end = 0xFFFFFFFF801FFFFF,
        .flags = PAGE_PRESENT | PAGE_WRITE | PAGE_EXECUTIVE,
        .name = "kernel_data"
    };

    // User space region
    struct memory_region user_space = {
        .start = 0x0000000000400000,
        .end = 0x0000000000800000,
        .flags = PAGE_PRESENT | PAGE_WRITE | PAGE_USER,
        .name = "user_space"
    };
}
```

## Advanced Memory Management

### Memory Pools

```c
// Create specialized memory pools for frequent allocations
struct memory_pool {
    void *base;
    size_t block_size;
    size_t total_blocks;
    size_t free_blocks;
    uint32_t *free_bitmap;
};

struct memory_pool *create_memory_pool(size_t block_size, size_t count) {
    struct memory_pool *pool = kmalloc(sizeof(struct memory_pool));
    if (!pool) return NULL;

    pool->base = kmalloc_page(block_size * count);
    pool->block_size = block_size;
    pool->total_blocks = count;
    pool->free_blocks = count;
    pool->free_bitmap = kcalloc((count + 31) / 32, sizeof(uint32_t));

    return pool;
}

void *pool_alloc(struct memory_pool *pool) {
    if (pool->free_blocks == 0) return NULL;

    // Find first free block
    for (size_t i = 0; i < pool->total_blocks; i++) {
        size_t bitmap_index = i / 32;
        size_t bit_index = i % 32;

        if (!(pool->free_bitmap[bitmap_index] & (1 << bit_index))) {
            pool->free_bitmap[bitmap_index] |= (1 << bit_index);
            pool->free_blocks--;
            return pool->base + (i * pool->block_size);
        }
    }

    return NULL;
}
```

### Memory Debugging

```c
// Debug memory allocation tracking
struct alloc_info {
    void *ptr;
    size_t size;
    const char *file;
    int line;
    struct alloc_info *next;
};

static struct alloc_info *alloc_list = NULL;
static spinlock_t alloc_lock = {0};

void *debug_kmalloc(size_t size, const char *file, int line) {
    void *ptr = kmalloc(size);
    if (!ptr) return NULL;

    spinlock_acquire(&alloc_lock);

    struct alloc_info *info = kmalloc(sizeof(struct alloc_info));
    if (info) {
        info->ptr = ptr;
        info->size = size;
        info->file = file;
        info->line = line;
        info->next = alloc_list;
        alloc_list = info;
    }

    spinlock_release(&alloc_lock);
    return ptr;
}

void debug_kfree(void *ptr) {
    if (!ptr) return;

    spinlock_acquire(&alloc_lock);

    struct alloc_info **current = &alloc_list;
    while (*current) {
        if ((*current)->ptr == ptr) {
            struct alloc_info *to_remove = *current;
            *current = (*current)->next;
            kfree(to_remove);
            break;
        }
        current = &(*current)->next;
    }

    spinlock_release(&alloc_lock);
    kfree(ptr);
}

#define kmalloc(size) debug_kmalloc(size, __FILE__, __LINE__)
#define kfree(ptr) debug_kfree(ptr)
```

## Best Practices

1. **Always check return values** - Memory allocation can fail
2. **Free allocated memory** - Prevent memory leaks
3. **Use appropriate allocation sizes** - Avoid fragmentation
4. **Consider alignment** - Use page-aligned allocation when needed
5. **Protect sensitive memory** - Use proper page protection flags

## Memory Layout

### Kernel Space Layout

```
0xFFFFFFFF80000000 - 0xFFFFFFFF9FFFFFFF : Kernel code and data
0xFFFFFFFFA0000000 - 0xFFFFFFFFBFFFFFFF : Kernel heap
0xFFFFFFFFC0000000 - 0xFFFFFFFFFEFFFFFF : Device mappings
0xFFFFFFFFFF000000 - 0xFFFFFFFFFFFFFFFF : Fixed mappings
```

### User Space Layout

```
0x0000000000000000 - 0x0000000000400000 : Reserved
0x0000000000400000 - 0x0000000000800000 : Program text
0x0000000000800000 - 0x0000000000C00000 : Program data
0x0000000000C00000 - 0x000000007FFFFFFF : Program heap
0x000000007FFFF000 - 0x00007FFFFFFFFFFF : Stack and libraries
```

## Error Handling

Memory management functions provide clear error indicators:

```c
void robust_memory_allocation(void) {
    void *buffer = kmalloc(1024);
    if (!buffer) {
        printf("Memory allocation failed - out of memory\n");
        // Handle out-of-memory condition
        return;
    }

    // Use buffer...

    kfree(buffer);
}

int safe_memory_operation(void) {
    uint64_t phys_page = pmm_alloc_page();
    if (phys_page == 0) {
        return -ENOMEM;  // No memory available
    }

    int result = vmm_map_page(VIRT_ADDRESS, phys_page, PAGE_PRESENT | PAGE_WRITE);
    if (result != 0) {
        pmm_free_page(phys_page);
        return result;  // Mapping failed
    }

    // Success
    return 0;
}
```

## Performance Considerations

- **Minimize fragmentation** - Use appropriate allocation sizes
- **Cache frequently used pages** - Keep hot data in memory
- **Use memory pools** - For frequent small allocations
- **Batch operations** - Allocate multiple pages when possible
- **Avoid frequent mapping/unmapping** - Keep mappings stable

## Integration Example

```c
#include "cane/mm.h"

void init_subsystem(void) {
    // Allocate memory for subsystem
    struct subsystem_data *data = kmalloc(sizeof(struct subsystem_data));
    if (!data) {
        printf("Failed to allocate subsystem data\n");
        return;
    }

    // Allocate working buffer
    data->buffer = kmalloc_page(SUBSYSTEM_BUFFER_SIZE);
    if (!data->buffer) {
        printf("Failed to allocate working buffer\n");
        kfree(data);
        return;
    }

    // Map hardware registers
    data->hardware_regs = mm_map_physical(HW_REGISTERS_BASE,
                                        HW_REGISTERS_SIZE,
                                        PAGE_PRESENT | PAGE_WRITE);
    if (!data->hardware_regs) {
        printf("Failed to map hardware registers\n");
        kfree(data->buffer);
        kfree(data);
        return;
    }

    // Initialize subsystem
    data->initialized = true;

    // Store for cleanup
    register_subsystem(data);
}

void cleanup_subsystem(struct subsystem_data *data) {
    if (!data) return;

    if (data->hardware_regs) {
        mm_unmap_physical(data->hardware_regs, HW_REGISTERS_SIZE);
    }

    if (data->buffer) {
        kfree(data->buffer);
    }

    kfree(data);
}
```

This memory management system provides a robust foundation for kernel and user space memory operations with proper protection, debugging, and performance optimization.
