/**
 * @file paging.c
 * @brief Hardware Page Table Management for x86_64 (Higher Half Updated).
 *
 * This module handles the creation and manipulation of the 4-level paging
 * hierarchy. All physical addresses are converted to higher-half virtual
 * addresses before access.
 */

#include <Valen/paging.h>
#include <Valen/pmm.h>
#include <Valen/stdio.h>
#include <Valen/spinlock.h>

/** * @brief The offset to shift physical addresses into the higher half.
 * Must match the value in boot.s and linker.ld.
 */
#define KERNEL_VIRT_OFFSET 0xFFFFFFFF80000000

/** * @brief Helper to convert a physical address to a virtual one the kernel can use.
 */
#define PHYS_TO_VIRT(phys) ((void *)((uint64_t)(phys) + KERNEL_VIRT_OFFSET))

/** * @brief Helper to extract the physical address from a page table entry.
 */
#define ENTRY_TO_PHYS(entry) ((uint64_t)(entry) & ~0xFFF)

/** @brief Pointer to the top-level Page Map Level 4 table. */
extern uint64_t p4_table[];

/** * @brief We must use the virtual address of the p4_table.
 * The linker now provides this as a higher-half address.
 */
uint64_t *kernel_pml4 = (uint64_t *)p4_table;

static spinlock_t paging_lock = SPINLOCK_INIT;

/**
 * @brief Initializes paging by ensuring the PML4 is loaded into CR3.
 */
void paging_init()
{
    if (!kernel_pml4)
    {
        printf("FATAL: No PML4 detected during paging_init\n");
        while (1)
            ;
    }

    /* CR3 requires a PHYSICAL address. We must subtract the offset. */
    uint64_t phys_pml4 = (uint64_t)kernel_pml4 - KERNEL_VIRT_OFFSET;

    asm volatile("mov %0, %%cr3" : : "r"(phys_pml4));
}

/**
 * @brief Maps a single 4KB virtual page to a physical address.
 *
 * @param virt The virtual address to map.
 * @param phys The physical address to map to.
 * @param flags The permission flags for the final page entry.
 */
void paging_map(uint64_t virt, uint64_t phys, uint64_t flags)
{
    spinlock_acquire(&paging_lock);
    
    uint64_t pml4_idx = (virt >> 39) & 0x1FF;
    uint64_t pdpt_idx = (virt >> 30) & 0x1FF;
    uint64_t pd_idx = (virt >> 21) & 0x1FF;
    uint64_t pt_idx = (virt >> 12) & 0x1FF;

    /* 1. PML4 -> PDPT */
    if (!(kernel_pml4[pml4_idx] & 1))
    {
        uint64_t new_phys = (uint64_t)pmm_alloc_page();
        uint64_t *pdpt = (uint64_t *)PHYS_TO_VIRT(new_phys);
        for (int i = 0; i < 512; i++)
            pdpt[i] = 0;

        kernel_pml4[pml4_idx] = new_phys | 0x07;
    }

    uint64_t *pdpt = (uint64_t *)PHYS_TO_VIRT(ENTRY_TO_PHYS(kernel_pml4[pml4_idx]));

    /* 2. PDPT -> PD */
    if (!(pdpt[pdpt_idx] & 1))
    {
        uint64_t new_phys = (uint64_t)pmm_alloc_page();
        uint64_t *pd = (uint64_t *)PHYS_TO_VIRT(new_phys);
        for (int i = 0; i < 512; i++)
            pd[i] = 0;

        pdpt[pdpt_idx] = new_phys | 0x07;
    }

    uint64_t *pd = (uint64_t *)PHYS_TO_VIRT(ENTRY_TO_PHYS(pdpt[pdpt_idx]));

    /* 3. PD -> PT */
    if (!(pd[pd_idx] & 1))
    {
        uint64_t new_phys = (uint64_t)pmm_alloc_page();
        uint64_t *pt = (uint64_t *)PHYS_TO_VIRT(new_phys);
        for (int i = 0; i < 512; i++)
            pt[i] = 0;

        pd[pd_idx] = new_phys | 0x07;
    }

    uint64_t *pt = (uint64_t *)PHYS_TO_VIRT(ENTRY_TO_PHYS(pd[pd_idx]));

    /* 4. PT -> Physical Page */
    pt[pt_idx] = (phys & ~0xFFF) | flags;
    
    spinlock_release(&paging_lock);
    /* Invalidate TLB */
    asm volatile("invlpg (%0)" ::"r"(virt) : "memory");
}

/**
 * @brief Maps a range of memory by iterating through pages.
 */
void paging_map_range(uint64_t virt, uint64_t phys, uint64_t size, uint64_t flags)
{
    for (uint64_t offset = 0; offset < size; offset += 4096)
    {
        paging_map(virt + offset, phys + offset, flags);
    }
}