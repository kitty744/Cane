/**
 * @file vmm.c
 * @brief Virtual Memory Manager (VMM) Implementation (Higher Half).
 */

#include <Valen/vmm.h>
#include <Valen/paging.h>
#include <Valen/pmm.h>
#include <Valen/stdio.h>
#include <Valen/spinlock.h>

#define KERNEL_VIRT_OFFSET 0xFFFFFFFF80000000ULL

#define PHYS_TO_VIRT(phys) ((void *)((uint64_t)(phys) + KERNEL_VIRT_OFFSET))
#define ENTRY_TO_PHYS(entry) ((uint64_t)(entry) & ~0xFFF)

static spinlock_t vmm_lock = SPINLOCK_INIT;


void vmm_init()
{
    paging_init();
}

/**
 * @brief Maps a virtual address to a physical address.
 */
void vmm_map(uintptr_t virt, uintptr_t phys, uint64_t flags)
{
    paging_map(virt, phys, flags);
    /* Invalidate TLB for the specific virtual address */
    asm volatile("invlpg (%0)" : : "r"(virt) : "memory");
}

/**
 * @brief Maps a contiguous range of memory.
 */
void vmm_map_range(uintptr_t virt, uintptr_t phys, uint64_t size, uint64_t flags)
{
    for (uint64_t i = 0; i < size; i += 4096)
    {
        paging_map(virt + i, phys + i, flags);
        asm volatile("invlpg (%0)" : : "r"(virt + i) : "memory");
    }
}

/**
 * @brief Allocates virtual pages and maps them to physical frames.
 */
void *vmm_alloc(uint64_t pages, uint64_t flags)
{
    spinlock_acquire(&vmm_lock);
    
    static uintptr_t next_alloc_addr = 0xFFFFFFFFB0000000;

    uintptr_t start_addr = next_alloc_addr;
    for (uint64_t i = 0; i < pages; i++)
    {
        void *phys = pmm_alloc_page();
        if (!phys)
        {
            spinlock_release(&vmm_lock);
            return 0;
        }
        paging_map(next_alloc_addr, (uintptr_t)phys, flags);
        asm volatile("invlpg (%0)" : : "r"(next_alloc_addr) : "memory");
        next_alloc_addr += 4096;
    }

    spinlock_release(&vmm_lock);
    return (void *)start_addr;
}

/**
 * @brief Translates a virtual address back to physical.
 */
uintptr_t vmm_get_phys(uintptr_t virtual_addr)
{
    uint64_t pml4_idx = (virtual_addr >> 39) & 0x1FF;
    uint64_t pdpt_idx = (virtual_addr >> 30) & 0x1FF;
    uint64_t pd_idx = (virtual_addr >> 21) & 0x1FF;
    uint64_t pt_idx = (virtual_addr >> 12) & 0x1FF;

    uint64_t cr3_val;
    asm volatile("mov %%cr3, %0" : "=r"(cr3_val));
    uint64_t *pml4 = (uint64_t *)PHYS_TO_VIRT(cr3_val & ~0xFFF);

    if (!(pml4[pml4_idx] & 1))
        return 0;

    uint64_t *pdpt = (uint64_t *)PHYS_TO_VIRT(ENTRY_TO_PHYS(pml4[pml4_idx]));
    if (!(pdpt[pdpt_idx] & 1))
        return 0;
    if (pdpt[pdpt_idx] & 0x80)
        return ENTRY_TO_PHYS(pdpt[pdpt_idx]) + (virtual_addr & 0x3FFFFFFF);

    uint64_t *pd = (uint64_t *)PHYS_TO_VIRT(ENTRY_TO_PHYS(pdpt[pdpt_idx]));
    if (!(pd[pd_idx] & 1))
        return 0;
    if (pd[pd_idx] & 0x80)
        return ENTRY_TO_PHYS(pd[pd_idx]) + (virtual_addr & 0x1FFFFF);

    uint64_t *pt = (uint64_t *)PHYS_TO_VIRT(ENTRY_TO_PHYS(pd[pd_idx]));
    if (!(pt[pt_idx] & 1))
        return 0;

    return ENTRY_TO_PHYS(pt[pt_idx]) + (virtual_addr & 0xFFF);
}