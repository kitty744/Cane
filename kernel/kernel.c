#include <cane/kernel.h>
#include <cane/stdio.h>
#include <cane/multiboot.h>
#include <cane/idt.h>
#include <cane/gdt.h>
#include <cane/pmm.h>
#include <cane/vmm.h>
#include <cane/heap.h>
#include <cane/shell.h>
#include <cane/keyboard.h>
 
int system_ready = 0;
 
#define KERNEL_VIRT_OFFSET 0xFFFFFFFF80000000ULL
#define PHYS_TO_VIRT(p) ((void *)((uint64_t)(p) + KERNEL_VIRT_OFFSET))
#define VIRT_TO_PHYS(v) ((uint64_t)(v) - KERNEL_VIRT_OFFSET)
 
extern char _kernel_end[];
 
void kmain(unsigned long magic, unsigned long addr)
{
    print_clear();

    idt_init();
    gdt_init();
    
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC)
    {
        while (1)
            ;
    }

    uint64_t max_physical_addr = 0;
    struct multiboot_tag_mmap *mmap_tag = NULL;

    struct multiboot_tag *tag = (struct multiboot_tag *)PHYS_TO_VIRT(addr + 8);
    while (tag->type != MULTIBOOT_TAG_TYPE_END)
    {
        if (tag->type == MULTIBOOT_TAG_TYPE_MMAP)
        {
            mmap_tag = (struct multiboot_tag_mmap *)tag;
            uint32_t entries = (mmap_tag->size - sizeof(struct multiboot_tag_mmap)) / mmap_tag->entry_size;
            for (uint32_t i = 0; i < entries; i++)
            {
                if (mmap_tag->entries[i].type == MULTIBOOT_MEMORY_AVAILABLE)
                {
                    uint64_t end = mmap_tag->entries[i].addr + mmap_tag->entries[i].len;
                    if (end > max_physical_addr)
                        max_physical_addr = end;
                }
            }
        }
        tag = (struct multiboot_tag *)((uint8_t *)tag + ((tag->size + 7) & ~7));
    }

    if (max_physical_addr == 0)
        max_physical_addr = 0x20000000;

    uintptr_t kernel_phys_end = VIRT_TO_PHYS((uintptr_t)_kernel_end);
    uintptr_t bitmap_phys = (kernel_phys_end + 0x1000) & ~0xFFFULL;
    pmm_init((uintptr_t)PHYS_TO_VIRT(bitmap_phys), max_physical_addr);

    if (mmap_tag)
    {
        uint32_t entries = (mmap_tag->size - sizeof(struct multiboot_tag_mmap)) / mmap_tag->entry_size;
        uint64_t bitmap_size = (max_physical_addr / 32768) + 4096;
        uintptr_t b_end = bitmap_phys + bitmap_size;

        for (uint32_t i = 0; i < entries; i++)
        {
            if (mmap_tag->entries[i].type == MULTIBOOT_MEMORY_AVAILABLE)
            {
                for (uint64_t a = mmap_tag->entries[i].addr; a < mmap_tag->entries[i].addr + mmap_tag->entries[i].len; a += 4096)
                {
                    if (a < 0x200000 || (a >= bitmap_phys && a < b_end))
                        continue;
                    pmm_mark_free(a);
                }
            }
        }
    }
 
    vmm_init();
    heap_init();
    keyboard_init();

    shell_init();
    
    system_ready = 1;
    
    /* Enable CPU interrupts */
    asm volatile ("sti");

    while (1) {
        asm volatile ("hlt");
    }
}