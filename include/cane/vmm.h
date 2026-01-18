#ifndef CANE_VMM_H
#define CANE_VMM_H

#include <stdint.h>

void vmm_init(void);
void *vmm_alloc(uint64_t size);
void vmm_free(void *ptr);
void vmm_map_page(uint64_t virt_addr, uint64_t phys_addr, uint64_t flags);
void vmm_unmap_page(uint64_t virt_addr);

#endif
