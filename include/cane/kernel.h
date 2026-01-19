#ifndef CANE_KERNEL_H
#define CANE_KERNEL_H

#include <stdint.h>

void kmain(unsigned long magic, unsigned long addr);
void panic(const char *msg);
void clear_screen(void);

#endif
