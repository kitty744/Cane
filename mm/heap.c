#include <cane/heap.h>
#include <cane/vmm.h>
#include <cane/pmm.h>
#include <cane/stdio.h>

#define HEAP_MAGIC 0x12345678

typedef struct heap_node
{
    uint32_t magic;
    uint64_t size;
    struct heap_node *next;
    int free;
} heap_node_t;

static heap_node_t *head = NULL;

void heap_init()
{
    void *first_page = vmm_alloc(4096, 0x03);
    if (!first_page)
    {
        return;
    }

    head = (heap_node_t *)first_page;
    head->magic = HEAP_MAGIC;
    head->size = 4096 - sizeof(heap_node_t);
    head->next = 0;
    head->free = 1;
}

void *malloc(uint64_t size)
{
    if (size == 0)
        return 0;
        
    size = (size + 7) & ~7;
    heap_node_t *curr = head;

    while (curr)
    {
        if (curr->free && curr->size >= size)
        {
            if (curr->size > size + sizeof(heap_node_t) + 32)
            {
                heap_node_t *new_node = (heap_node_t *)((uint8_t *)curr + sizeof(heap_node_t) + size);
                new_node->magic = HEAP_MAGIC;
                new_node->size = curr->size - size - sizeof(heap_node_t);
                new_node->next = curr->next;
                new_node->free = 1;

                curr->size = size;
                curr->next = new_node;
            }
            curr->free = 0;

            return (void *)((uint8_t *)curr + sizeof(heap_node_t));
        }

        if (!curr->next)
        {
            void *new_virt = vmm_alloc(4096, 0x03);
            if (!new_virt)
            {
                return 0;
            }

            heap_node_t *new_node = (heap_node_t *)new_virt;
            new_node->magic = HEAP_MAGIC;
            new_node->size = 4096 - sizeof(heap_node_t);
            new_node->next = 0;
            new_node->free = 1;
            curr->next = new_node;
        }
        curr = curr->next;
    }

    return 0;
}

void free(void *ptr)
{
    if (!ptr)
        return;

    heap_node_t *node = (heap_node_t *)((uint8_t *)ptr - sizeof(heap_node_t));

    if (node->magic != HEAP_MAGIC)
    {
        return;
    }

    node->free = 1;

    heap_node_t *temp = head;
    while (temp)
    {
        if (temp->free && temp->next && temp->next->free)
        {
            temp->size += sizeof(heap_node_t) + temp->next->size;
            temp->next = temp->next->next;
            continue;
        }
        
        if (temp->next && temp->next->free && temp->free)
        {
            temp->size += sizeof(heap_node_t) + temp->next->size;
            temp->next = temp->next->next;
            continue;
        }
        
        temp = temp->next;
    }
}
