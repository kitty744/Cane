/**
 * @file vmm.h
 * @brief Virtual Memory Manager Interface.
 */

#ifndef VMM_H
#define VMM_H

#include <stdint.h>
#include <stddef.h>

/** @brief Page Table Entry Flags (Standard x86_64) */
#define PAGE_PRESENT (1ULL << 0)
#define PAGE_WRITE (1ULL << 1)
#define PAGE_USER (1ULL << 2)
#define PAGE_PWT (1ULL << 3)  /* Page-level Write-Through */
#define PAGE_PCD (1ULL << 4)  /* Page-level Cache Disable (Required for MMIO) */
#define PAGE_HUGE (1ULL << 7) /* PS bit for 2MB/1GB pages */

/**
 * @brief Initializes the VMM and sets up initial kernel paging.
 */
void vmm_init();

/**
 * @brief Maps a virtual address to a physical address.
 * @param virt The destination virtual address.
 * @param phys The source physical address.
 * @param flags Architectural flags (PRESENT | WRITE | PCD etc).
 */
void vmm_map(uintptr_t virt, uintptr_t phys, uint64_t flags);

/**
 * @brief Maps a contiguous range of memory.
 */
void vmm_map_range(uintptr_t virt, uintptr_t phys, uint64_t size, uint64_t flags);

/**
 * @brief Translates virtual pointers to physical addresses for hardware/DMA.
 */
uintptr_t vmm_get_phys(uintptr_t virtual_addr);

/**
 * @brief Allocates virtual kernel memory and backs it with physical frames.
 */
void *vmm_alloc(uint64_t pages, uint64_t flags);

#endif