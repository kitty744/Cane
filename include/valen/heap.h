#ifndef HEAP_H
#define HEAP_H
#include <stdint.h>
#include <stddef.h>

void heap_init();
void *malloc(uint64_t size);
void free(void *ptr);

#endif