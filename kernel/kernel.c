#include <cane/kernel.h>
#include <cane/stdio.h>

void kmain(void) {
    clear_screen();
    printf("BOOT: SUCCESS\n");
    printf("CaneOS v0.1\n");
    printf("Memory Management: Loading...\n");
    
    while (1) {
        asm volatile ("hlt");
    }
}

void panic(const char *msg) {
    clear_screen();
    printf("PANIC: %s\n", msg);
    
    while (1) {
        asm volatile ("hlt");
    }
}
