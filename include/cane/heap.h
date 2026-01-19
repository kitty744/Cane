#ifndef HEAP_H
#define HEAP_H
#include <stdint.h>
#include <stddef.h>

void kheap_init();
void *kmalloc(uint64_t size);
void kfree(void *ptr);

#endif