#ifndef CANE_PMM_H
#define CANE_PMM_H

#include <stdint.h>

void pmm_init(void);
void *pmm_alloc(void);
void pmm_free(void *ptr);
uint64_t pmm_get_free_pages(void);

#endif
