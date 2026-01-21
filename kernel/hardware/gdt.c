#include <Valen/gdt.h>

struct gdt_entry gdt[3];
struct gdt_ptr gp;

extern void gdt_flush(uint64_t gdt_ptr);

void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F);

    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access = access;
}

void gdt_init()
{
    gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
    gp.base = (uint64_t)&gdt;

    // Entry 0: Null Descriptor
    gdt_set_gate(0, 0, 0, 0, 0);

    // Entry 1: Kernel Code
    // Access: 0x9A (10011010b) -> Present, Ring 0, Code, Readable
    // Flags:  0x20 (00100000b) -> L-Bit set (64-bit target)
    gdt_set_gate(1, 0, 0, 0x9A, 0x20);

    // Entry 2: Kernel Data
    // Access: 0x92 (10010010b) -> Present, Ring 0, Data, Writable
    gdt_set_gate(2, 0, 0, 0x92, 0x00);

    gdt_flush((uint64_t)&gp);
}