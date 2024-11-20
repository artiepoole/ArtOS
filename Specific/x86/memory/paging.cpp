//
// Created by artypoole on 18/11/24.
//

#include "memory.h"
#include "multiboot2.h"
#include <logging.h>

constexpr size_t base_address_shift = 12;
constexpr size_t page_alignment = 4096;

/// Each table is 4k in size, and is page aligned i.e. 4k aligned. They consists of 1024 32 bit entries.
constexpr size_t page_table_size = 1024;

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
    page_table_entry table[page_table_size];
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

page_directory_4kb_t paging_directory[1024]__attribute__((aligned(page_alignment)));
page_table paging_tables[page_table_size]__attribute__((aligned(page_alignment)));

// 4GB worth of 4096 pages. Set all to false. Processing the multiboot2 memory_map will set the necessary bits.
bool page_available_bitmap[0x100000] = {false};

// TODO: helper functions like page_alloc, page_free, page_get_next_addr, converters between addresses and page numbers?

size_t page_get_next_addr()
{
    size_t idx = 0;
    while (!page_available_bitmap[idx++])
    {
    }
    return idx;
}

// void* page_alloc()
// {
// u32 physical_addr = next_page_start;
// for (size_t page = 0; page < n_pages; page++)
// {
//     if (page % 1024 == 0)
//     {
//         // every 1024 pages, a new table is used. This sets the next page dir entry to point to the next table.
//         page_directory_4kb_t new_dir_entry = {
//             1,
//             1,
//             1,
//             0,
//             0,
//             0,
//             0,
//             0,
//             0,
//             reinterpret_cast<uintptr_t>(&paging_tables[page / 1024]) >> 12,
//         };
//         paging_directory[page / 1024] = new_dir_entry;
//     }
//     const page_table_entry new_entry{
//         1,
//         1,
//         1,
//         0,
//         0,
//         0,
//         0,
//         0,
//         0,
//         0,
//         physical_addr >> 12
//     };
//     // new_entry.raw = 7; // set present, RW and User
//     // new_entry.physical_address = physical_addr
//     physical_addr += 4096;
//     paging_tables[page / 1024].table[page % 1024] = new_entry;
// }
// }

// void page_free(const uintptr_t vaddr)
// {
//
// }


void identity_map(uintptr_t phys_addr, const size_t size, const bool writable, const bool user)
{
    virtual_address_t virtual_address = {};
    virtual_address.raw = phys_addr;
    size_t dir_idx = -1;
    for (size_t i = 0; i < size / page_alignment; i++)
    {
        // Every 1024 table entries requires a new dir entry.
        if (virtual_address.page_directory_index != dir_idx)
        {
            dir_idx = virtual_address.page_directory_index;
            page_directory_4kb_t dir_entry;
            dir_entry.page_table_entry_address = reinterpret_cast<uintptr_t>(&paging_tables[dir_idx]) >> base_address_shift; // might be wrong.
            dir_entry.rw = writable;
            dir_entry.user_access = user;
            paging_directory[dir_idx] = dir_entry;
        }

        page_table_entry tab_entry = {};
        tab_entry.present = true;
        tab_entry.physical_address = phys_addr >> base_address_shift;
        tab_entry.rw = writable;
        tab_entry.user_access = user;

        paging_tables[virtual_address.page_directory_index].table[virtual_address.page_table_index] = tab_entry;

        phys_addr += page_alignment;
        virtual_address.raw = phys_addr;

        page_available_bitmap[phys_addr / page_alignment] = false;
    }
}

void enable_paging()
{
    auto addr = reinterpret_cast<u32>(&paging_directory[0]) | !0x1000;
    __asm__ volatile ("mov %0, %%cr3" : : "r"(addr)); // set the cr3 to the paging_directory physical address
    virtual_address_t cr2;
    cr2.raw = 0x0020db7f;
    u32 cr0 = 0;
    __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 = cr0 | 0x80000001;
    __asm__ volatile ("mov %0, %%cr0" : : "r"(cr0));

    return;
    // auto addr = reinterpret_cast<u32>(&paging_directory[0]) | !0x1000;
    // __asm__ volatile ("movl %0, %%cr3" : : "r"(addr));
    // // __asm__ volatile ("mov %0, %%cr3" : : "=r"(addr)); // set the cr3 to the paging_directory physical address
    //
    // u32 cr0 = 0;
    // __asm__ volatile ("smsw %0" : "=r"(cr0));
    // cr0 = cr0 | 0x80000001;
    // __asm__ volatile ("lmsw %0" : : "m"(cr0));
}

/*
 *  uses kernel_brk because this is the last memory address to be given to the kernel.
 *
 */
void mmap_init(multiboot2_tag_mmap* mmap)
{
    const auto brk_loc = reinterpret_cast<uintptr_t>(kernel_brk);
    const size_t n_entries = mmap->size / sizeof(multiboot2_mmap_entry);

    // TODO: replace with calls to identity_map(...) to map all already assigned memory
    for (size_t i = 0; i < n_entries; i++)
    {
        multiboot2_mmap_entry const* entry = mmap->entries[i];
        if (entry->type != 1)
        {
            // Any type except 1 is reserved memory.
            identity_map(entry->addr, entry->len, false, false);
            continue;
        }
        if (entry->addr + entry->len < brk_loc || entry->addr > brk_loc)
        {
            // This region is free memory, but doesn't contain the kernel. Therefore, this is probably the BIOS region.
            // This needs to be identity mapped, and so I map it with RW and supervisor only.
            // Technically, this could be made available but some of this memory might be used in the
            // kernel boot sequence i.e. may contain the framebuffer. That's why it is also set to writable.
            //
            // This could also be a second section of free memory and so this approach may be stupid.
            identity_map(entry->addr, entry->len, true, false);
            continue;
        }

        LOG("Found region including kernel");
        main_region_start = entry->addr;
        main_region_end = entry->addr + entry->len;
        next_page_start = (brk_loc & 0xFFFFF000) + 0x1000; // 4k aligned. 32 bit max address. As the Kernel expands, this may want to be larger so that kernel stuff can be incremented with sbrk instead
        kernel_brk = reinterpret_cast<u8*>(next_page_start);
        user_space_memory_available = main_region_end - next_page_start;

        // Protect kernel and init identity map
        identity_map(main_region_start, next_page_start - main_region_start, true, false);

        // set upper limit in bitmap to extents of this memory region.
        for (size_t addr = next_page_start; addr < main_region_end; addr += page_alignment)
        {
            page_available_bitmap[addr / page_alignment] = true;
        }

        // TODO: pass the address of the table to CPU
        // TODO: enter paging mode
        // TODO: this should not return. This process should handle ALL regions.
    }


    LOG("Memory map processed.");
    enable_paging();
    LOG("Paging enabled.");
}
