#include <stdint.h>
#include <cane/stdio.h>
#include <cane/panic.h>
#include <cane/vga.h>

/**
 * @brief Panic-level Page Fault Handler.
 */
void page_fault_handler(uint64_t error_code)
{
    print_clear();
    set_color(VGA_COLOR_LIGHT_RED);
    
    uint64_t fault_addr;
    asm volatile("mov %%cr2, %0" : "=r"(fault_addr));

    printf("\n--- FATAL PAGE FAULT ---\n");
    printf("Address: ");
    print_hex(fault_addr);
    printf("\nError Code: ");
    print_int(error_code);

    /* Decode error code for debugging */
    if (error_code & 1)
        printf(" [Protection Violation]");
    else
        printf(" [Non-present Page]");

    if (error_code & 2)
        printf(" [Write]");
    else
        printf(" [Read]");

    if (error_code & 4)
        printf(" [User Mode]");
    else
        printf(" [Kernel Mode]");

    printf("\nSystem Halted.");
    while (1)
        asm volatile("hlt");
}