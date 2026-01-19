#ifndef PMM_H
#define PMM_H

#include <stdint.h>

void pmm_init(uintptr_t start, uint64_t size);
void pmm_mark_free(uintptr_t addr);
void pmm_mark_used(uintptr_t addr);
void *pmm_alloc_page();
void pmm_free_page(void *addr);

uint64_t pmm_get_total_kb();
uint64_t pmm_get_used_kb();
uint64_t pmm_get_free_kb();

#endif