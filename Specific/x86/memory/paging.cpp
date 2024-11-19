//
// Created by artypoole on 18/11/24.
//

#include "memory.h"
#include "multiboot2.h"
#include <logging.h>


union virtual_address_t
{
    struct
    {
        u32 page_offset : 12;
        u32 page_table_index : 10;
        u32 page_directory_index : 10;
    };

    u32 raw;
};

union page_directory_4kb_t
{
    struct
    {
        u32 present : 1 = true;
        u32 rw : 1; // read/write if set, read-only otherwise
        u32 user_access : 1; // user and supervisor if set, supervisor only if not.
        u32 write_through : 1 = false; // write through caching if set, write-back otherwise
        u32 cache_disable : 1 = false; // is caching disabled?
        u32 accessed : 1 = false; // Gets sets on access, has to be cleared manually by OS if used.
        u32 OS : 1 = false; // free bit available for OS use
        u32 extended_page_size : 1 = false; // 4MB pages if set, 4KB if not. (Always not set in ArtOS).
        u32 OS_data : 4 = 0; // free nibble available for OS flags
        u32 page_table_entry_address : 20; // bits 12-31. 4KiB alignment means first 12 bits are always zero.
    };

    u32 raw;
};

union page_table_entry
{
    struct
    {
        u32 present : 1 = true;
        u32 rw : 1; // read write if set, read only otherwise
        u32 user_access : 1; // user and supervisor if set, supervisor only if not.
        u32 write_through : 1 = false; // write through caching if set, write-back otherwise
        u32 cache_disable : 1 = false; // is caching disabled?
        u32 accessed : 1 = false; // Gets sets on access, has to be cleared manually by OS if used.
        u32 dirty : 1 = false; // "Has been written to if set"
        u32 page_attribute_table : 1 = false; // Res and zero probably: "If PAT is supported, then PAT along with PCD and PWT shall indicate the memory caching type. Otherwise, it is reserved and must be set to 0."
        u32 global : 1 = false; // "If PAT is supported, then PAT along with PCD and PWT shall indicate the memory caching type. Otherwise, it is reserved and must be set to 0."
        u32 OS_data : 3 = 0; // free nibble available for OS flags
        u32 physical_address : 20; // bits 12-31. 4KiB alignment means first 12 bits are always zero.
    };

    u32 raw;
};

struct page_table
{
    page_table_entry table[1024];
};

// For now user space memory regions are assumed contiguous, but really this will need to be some kind of map or linked list.
struct assigned_memory
{
    void* owner; // TODO: how to store the parent process
    uintptr_t start;
    size_t n_pages;
};

uintptr_t main_region_start;
size_t user_space_memory_available;
uintptr_t main_region_end;
uintptr_t next_page_start;

page_directory_4kb_t paging_directory[1024]__attribute__((aligned(1024 * 4)));
page_table paging_tables[1024]__attribute__((aligned(1024 * 4)));

/*
 *  uses kernel_brk because this is the last memory address to be given to the kernel.
 *
 */
void mmap_init(multiboot2_tag_mmap* mmap)
{
    auto brk_loc = reinterpret_cast<uintptr_t>(kernel_brk);
    size_t n_entries = mmap->size / sizeof(multiboot2_mmap_entry);
    for (int i = 0; i < n_entries; i++)
    {
        multiboot2_mmap_entry const* entry = mmap->entries[i];
        if (entry->type != 1)
        {
            continue;
        }
        if (entry->addr + entry->len < brk_loc || entry->addr > brk_loc)
        {
            continue;
        } // entry is before the kernel. This is not a free region, this is a bios reserved region which hsa been freed.

        LOG("Found region including kernel");
        main_region_start = entry->addr;
        main_region_end = entry->addr + entry->len;
        next_page_start = (brk_loc & 0xFFFFF000) + 0x1000; // 4k aligned. 32 bit max address. As the Kernel expands, this may want to be larger so that kernel stuff can be incremented with sbrk instead
        user_space_memory_available = main_region_end - next_page_start;
        u32 n_pages = user_space_memory_available / 4096;


        /* TODO: THIS SHOULD ACTUALLY ONLY MAP EVERYTHING ELSE BUT IT SHOULD DO IT IDENTITY WISE
         * This is because all the pointers assigned so far are physical address values, but after enabling paging
         * these will be interpreted as virtual memory addresses.
         */
        u32 physical_addr = next_page_start;
        for (size_t page = 0; page < n_pages; page++)
        {
            if (page % 1024 == 0)
            {
                // every 1024 pages, a new table is used. This sets the next page dir entry to point to the next table.
                page_directory_4kb_t new_dir_entry = {
                    1,
                    1,
                    1,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    reinterpret_cast<uintptr_t>(&paging_tables[page / 1024]) >> 12,
                };
                paging_directory[page / 1024] = new_dir_entry;
            }
            const page_table_entry new_entry{
                1,
                1,
                1,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                physical_addr >> 12
            };
            // new_entry.raw = 7; // set present, RW and User
            // new_entry.physical_address = physical_addr
            physical_addr += 4096;
            paging_tables[page / 1024].table[page % 1024] = new_entry;
        }

        // TODO: pass the address of the table to CPU
        return;
    }

    LOG("ERROR: Failed to find a memory region containing the kernel. Memory mapping will not work.");
}
