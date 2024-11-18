//
// Created by artypoole on 18/11/24.
//

#include "memory.h"
#include "multiboot2.h"
#include <logging.h>

// For now user space memory regions are assumed contiguous, but really this will need to be some kind of map or linked list.
struct assigned_memory
{
    void* owner; // TODO: how to store the parent
    uintptr_t start;
    size_t n_pages;
};

uintptr_t main_region_start;
size_t user_space_memory_available;
uintptr_t main_region_end;
uintptr_t next_page_start;

/*
 *  uses kernel_brk because this is the last memory address to be given to the kernel.
 *
 */
void mmap_init(multiboot2_tag_mmap* mmap)
{
    auto brk = reinterpret_cast<uintptr_t>(&kernel_brk);

    size_t n_entries = mmap->size / sizeof(multiboot2_mmap_entry);
    for (int i = 0; i < n_entries; i++)
    {
        if (multiboot2_mmap_entry* entry = mmap->entries[i]; entry->type == 1)
        {
            if (entry->addr + entry->len < brk || entry->addr > brk)
            {
                continue;
            } // entry is before the kernel. This is not a free region, this is a bios reserved region which hsa been freed.
            LOG("Found region including kernel");
            main_region_start = entry->addr;
            main_region_end = entry->addr + entry->len;
            next_page_start = (brk & 0xFFFFF000) + 0x1000; // 4k aligned. 32 bit max address. As the Kernel expands, this may want to be larger so that kernel stuff can be incremented with sbrk instead
            user_space_memory_available = main_region_end - next_page_start;
            break;
        }
    }
    LOG("ERROR: Failed to find a memory region containing the kernel. Memory mapping will not work.");
}
