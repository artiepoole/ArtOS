//
// Created by artypoole on 18/11/24.
//

#include "memory.h"
#include "multiboot2.h"
#include "logging.h"
#include "string.h"
#include "cmp_int.h"

constexpr size_t base_address_shift = 12;
constexpr size_t page_alignment = 4096;

/// Each table is 4k in size, and is page aligned i.e. 4k aligned. They consists of 1024 32 bit entries.
constexpr size_t page_table_size = 1024;

union virtual_address_t
{
    uintptr_t raw;

    struct
    {
        uintptr_t page_offset : 12;
        uintptr_t page_table_index : 10;
        uintptr_t page_directory_index : 10;
    };
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
uintptr_t main_region_end;
uintptr_t next_page_start;

page_directory_4kb_t paging_directory[page_table_size]__attribute__((aligned(page_alignment)));
page_table paging_tables[page_table_size]__attribute__((aligned(page_alignment)));

// bitmaps used to keep track of the next virtual and physical pages available.
// These are the same during identity mapping, but diverge when user space programs make malloc calls
// 4GB worth of 4096 pages. Set all to false. Processing the multiboot2 memory_map will set the necessary bits.
bool page_available_physical_bitmap[0x100000] = {false};
bool page_available_virtual_bitmap[0x100000] = {false};

// TODO: helper functions like page_alloc, page_free, page_get_next_addr, converters between addresses and page numbers?

uintptr_t page_get_next_phys_addr()
{
    size_t idx = 0;
    for (; !page_available_physical_bitmap[idx] && idx < 0x100000; idx++)
    {
    }
    return idx * page_alignment;
}

uintptr_t page_get_next_virtual_chunk(size_t idx, const size_t n_pages)
{
    bool looking = true;
    while (looking)
    {
        size_t i = 1;
        while (!page_available_virtual_bitmap[idx])
        {
            idx++;
        }
        while (page_available_virtual_bitmap[idx + i] && i < n_pages)
        {
            i++;
        }
        if (i == n_pages) { looking = false; }
        else { idx += i; }
    }
    return idx * page_alignment;
}

// Should go back to 0 if doesn't find anything.
uintptr_t page_get_next_virt_addr(const uintptr_t start_addr)
{
    size_t idx = start_addr >> base_address_shift;
    for (; !page_available_virtual_bitmap[idx] && idx < 0x100000; idx++)
    {
    }
    return idx * page_alignment;
}


void assign_page_directory_entry(size_t dir_idx, const bool writable, const bool user)
{
    page_directory_4kb_t dir_entry;
    dir_entry.page_table_entry_address = reinterpret_cast<uintptr_t>(&paging_tables[dir_idx]) >> base_address_shift; // might be wrong.
    dir_entry.rw = writable;
    dir_entry.user_access = user;
    paging_directory[dir_idx] = dir_entry;
}

void assign_page_table_entry(const uintptr_t physical_addr, const virtual_address_t v_addr, const bool writable, const bool user)
{
    page_table_entry tab_entry = {};
    tab_entry.present = true;
    tab_entry.physical_address = physical_addr >> base_address_shift;
    tab_entry.rw = writable;
    tab_entry.user_access = user;

    paging_tables[v_addr.page_directory_index].table[v_addr.page_table_index] = tab_entry;

    page_available_physical_bitmap[physical_addr >> base_address_shift] = false;
    page_available_virtual_bitmap[v_addr.raw >> base_address_shift] = false;
}

// TODO: do we need to unassign directories? Probably not.

int unassign_page_table_entries(const size_t start_idx, const size_t n_pages)
{
    for (size_t i = start_idx; i < start_idx + n_pages; i++)
    {
        auto* tab_entry = &paging_tables[i / 1024].table[i % 1024];
        if (!tab_entry->present) return -1;
        const size_t phys_idx = tab_entry->physical_address;
        page_available_virtual_bitmap[i] = true;
        page_available_physical_bitmap[phys_idx] = true;
        tab_entry->rw = 0;
    }
    return 0;
}


void paging_identity_map(uintptr_t phys_addr, const size_t size, const bool writable, const bool user)
{
    virtual_address_t virtual_address = {phys_addr};
    size_t dir_idx = -1;
    const size_t num_pages = (size + page_alignment - 1) >> base_address_shift;
    for (size_t i = 0; i < num_pages; i++)
    {
        // Every 1024 table entries requires a new dir entry.
        if (virtual_address.page_directory_index != dir_idx)
        {
            assign_page_directory_entry(virtual_address.page_directory_index, writable, user);
        }

        assign_page_table_entry(phys_addr, virtual_address, writable, user);

        phys_addr += page_alignment;
        virtual_address.raw = phys_addr;
    }
}

void enable_paging()
{
    auto addr = reinterpret_cast<u32>(&paging_directory[0]) | !0x1000;
    __asm__ volatile ("mov %0, %%cr3" : : "r"(addr)); // set the cr3 to the paging_directory physical address
    u32 cr0 = 0;
    __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 = cr0 | 0x80000001;
    __asm__ volatile ("mov %0, %%cr0" : : "r"(cr0));
}

/*
 *  Uses kernel_brk because this is the last memory address to be given to the kernel.
 *  This could use kernel end once sbrk is no longer a thing.
 */
void mmap_init(multiboot2_tag_mmap* mmap)
{
    memset(page_available_virtual_bitmap, true, sizeof(bool) * 0x100000);
    const auto brk_loc = reinterpret_cast<uintptr_t>(kernel_brk);
    const size_t n_entries = mmap->size / sizeof(multiboot2_mmap_entry);

    // TODO: replace with calls to paging_identity_map(...) to map all already assigned memory
    for (size_t i = 0; i < n_entries; i++)
    {
        multiboot2_mmap_entry const* entry = mmap->entries[i];
        if (entry->type != 1)
        {
            // Any type except 1 is reserved memory. If not hardware mapped then set to read only.
            paging_identity_map(entry->addr, entry->len, false, false);
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
            // fill the bios hole
            extern unsigned char kernel_start;
            const size_t start = MIN(0, entry->addr);
            paging_identity_map(start, MAX(entry->len+entry->addr, reinterpret_cast<size_t>(&kernel_start)) - start, true, false);

            continue;
        }

        LOG("Paging: found region including kernel");
        // TODO: these can be local variables/not used at all if sbrk is not used.
        main_region_start = entry->addr;
        main_region_end = entry->addr + entry->len;
        next_page_start = (brk_loc & 0xFFFFF000) + 0x1000; // 4k aligned. 32 bit max address.
        kernel_brk = reinterpret_cast<u8*>(next_page_start);

        // Protect kernel and init identity map
        paging_identity_map(main_region_start, next_page_start - main_region_start, true, false);

        // set upper limit in bitmap to extents of this memory region.
        for (size_t addr = next_page_start; addr < main_region_end; addr += page_alignment)
        {
            page_available_physical_bitmap[addr >> base_address_shift] = true;
        }

        // manual region
    }

    // TODO: consider a hole filling algorithm to fix the need for manual identity mapping of framebuffer and APIC stuff.
    virtual_address_t virtual_address = {0xFC000000};
    LOG("Paging: memory map processed.");
    enable_paging();
    LOG("Paging: paging enabled.");
}


void* mmap(uintptr_t addr, size_t length, int prot, int flags, int fd, size_t offset)
{
    // mmap should return a contiguous chunk of virtual memory, so some checks should be done first.
    // if (addr < main_region_start) addr = main_region_start;

    size_t first_page = addr >> base_address_shift;
    const size_t num_pages = (length + page_alignment - 1) >> base_address_shift;

    virtual_address_t ret_addr = {page_get_next_virtual_chunk(first_page, num_pages)};
    virtual_address_t working_addr = ret_addr;
    for (size_t i = 0; i < num_pages; i++)
    {
        if (!paging_directory[working_addr.page_directory_index].present)
        {
            // TODO: handle flag ints as bools here
            assign_page_directory_entry(working_addr.page_directory_index, true, false);
        }

        assign_page_table_entry(
            page_get_next_phys_addr(),
            working_addr,
            true,
            false
        );

        working_addr.raw += page_alignment;
    }
    return reinterpret_cast<void*>(ret_addr.raw);
}

int munmap(void* addr, const size_t length)
{
    // TODO: More checks such as "you don't own this memory"
    return unassign_page_table_entries(
        reinterpret_cast<uintptr_t>(addr) >> base_address_shift,
        (length + page_alignment - 1) >> base_address_shift);
}
