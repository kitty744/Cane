#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

#define PAGE_PRESENT (1ULL << 0)
#define PAGE_WRITE (1ULL << 1)
#define PAGE_USER (1ULL << 2)
#define PAGE_PWT (1ULL << 3) // Page Write-Through
#define PAGE_PCD (1ULL << 4)  // Page-level Cache Disable
#define PAGE_HUGE (1ULL << 7) /* PS bit for 2MB/1GB pages */

void paging_init();
void paging_map(uint64_t virt, uint64_t phys, uint64_t flags);
void paging_map_range(uint64_t virt, uint64_t phys, uint64_t size, uint64_t flags);
void paging_map_page(uint64_t virt, uint64_t phys, uint64_t flags);

#endif
