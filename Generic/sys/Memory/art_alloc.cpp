// ArtOS - hobby operating system by Artie Poole
// Copyright (C) 2025 Stuart Forbes Poole <artiepoole>
//
//     This program is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with this program.  If not, see <https://www.gnu.org/licenses/>

//
// Created by artiepoole on 3/12/25.
//

#include <stdlib.h>
#include <doomgeneric/p_local.h>

#include "LinkedList.h"
#include "memory.h"


enum ART_ALLOC_FLAGS
{
    ALIGNED_4 = 1 << 2,
    ALIGNED_8 = 1 << 3,
    ALIGNED_4096 = 1 << 9, // full page alignment
};


namespace art_allocator
{
    size_t MIN_CHUNK_SIZE = 64; // arbitrary
    struct chunk_t
    {
        void* start; // address in bytes
        size_t size; // in bytes
        size_t prev;
        size_t next;
        bool memory_free : 1;
        bool array_free : 1;
    };

    // for the array we need to store the max idx and

    // TODO: Woody says I should rewrite the LL method from scratch to be more suitable for malloc: allow access to prev and next.

    chunk_t* chunks;
    size_t n_chunks = 0;
    size_t next_free_array_entry = 0;
    size_t end_chunk_idx = 0;

    size_t idx_from_ptr(const chunk_t* chunk)
    {
        return chunk - chunks;
    }


    bool expand_chunk_array()
    {
        const size_t new_size = n_chunks * 2;
        const size_t n_pages = (new_size * sizeof(chunk_t) + page_alignment - 1) / page_alignment;

        auto* new_chunks = static_cast<chunk_t*>(mmap(0, n_pages * page_alignment, 0, 0, 0, 0));

        if (not new_chunks) return false;

        memcpy(new_chunks, chunks, sizeof(chunk_t) * n_chunks);
        // NOTE: always a full page so munmap is safe.
        munmap(chunks, sizeof(chunk_t) * n_chunks);
        chunks = new_chunks;
        n_chunks = new_size;
        return true;
    }

    // This is for getting where to store the new chunk info after getting new pages, or when splitting a chunk to assign part of a chunk
    size_t get_next_free_array_entry()
    {
        size_t idx = next_free_array_entry;
        while (true)
        {
            while (idx < n_chunks)
            {
                if (chunks[idx].array_free) return idx;
                ++idx;
            }
            expand_chunk_array(); // Expand the array and try again
        }
    }

    // frees a chunk in the array
    void deallocate_chunk_entry(size_t idx)
    {
        memset(&chunks[idx], 0, sizeof(chunk_t));
        if (idx < next_free_array_entry) next_free_array_entry = idx;
    }


    // Just stores a new chunk in next available array idx. Returns the index used to store this data
    size_t append_chunk(chunk_t const& data)
    {
        // NORMAL BEHAVIOUR:
        const size_t idx = next_free_array_entry;
        chunks[idx] = data;
        next_free_array_entry = get_next_free_array_entry();
        return idx;
    }

    // This needs to handle writing the chunk info the array
    size_t get_pages_for_new_chunk(const size_t size_bytes)
    {
        const size_t n_pages = (size_bytes + page_alignment - 1) / page_alignment;
        const size_t got_bytes = n_pages * page_alignment;
        const auto ptr = mmap(0, n_pages * page_alignment, 0, 0, 0, 0);
        if (!ptr)
        {
            exit(-1);
        }
        if (chunk_t end_chunk = chunks[end_chunk_idx]; end_chunk.start + end_chunk.size == ptr) // is it contiguous
        {
            end_chunk.size += got_bytes;
            return end_chunk_idx;
        }
        // assumes always adding to end:
        const chunk_t chunk_data = {ptr, got_bytes, end_chunk_idx, next_free_array_entry, true, true};
        const size_t chunk_idx = append_chunk(chunk_data);
        end_chunk_idx = chunk_idx;

        return chunk_idx;
    }

    // This gets the next chunk that's big enough or makes a new one
    size_t get_suitable_chunk(const size_t size_bytes)
    {
        size_t idx = 0;
        while (idx < n_chunks)
        {
            if (chunks[idx].memory_free & chunks[idx].size >= size_bytes) return idx;
            ++idx;
        }

        return get_pages_for_new_chunk(size_bytes);
    }

    void split_chunk(const size_t chunk_idx, const size_t size_bytes)
    {
        const size_t latter = get_next_free_array_entry();
        chunks[latter].next = chunks[chunk_idx].next;
        chunks[chunk_idx].next = latter;
        chunks[chunk_idx].memory_free = false;
        chunks[latter].size = chunks[chunk_idx].size - size_bytes;
        chunks[chunk_idx].size = size_bytes;
        chunks[latter].prev = chunk_idx;
    }

    void merge_chunk(const size_t prior, const size_t latter)
    {
        chunks[prior].next = chunks[latter].next;
        chunks[prior].size += chunks[latter].size;
        deallocate_chunk_entry(latter);
    }
}

// Hmmm how do we handle available memory? Do we create one chunk for all memory and then allocate as and
// when it's free or do we just append new chunks as and when we need them and let mmap do all the hard part?

// NOTE: the chunks will always be in order so best to scan forwards and backwards wherever it makes sense to
// (once doubly linked is implemented)

using namespace art_allocator;


/** Initialises memory area by fetching a single page to be used to store chunk info.
 */
void art_memory_init()
{
    constexpr size_t n_pages = 1;
    constexpr size_t new_size = page_alignment / sizeof(chunk_t);;
    auto* new_chunks = static_cast<chunk_t*>(mmap(0, n_pages * page_alignment, 0, 0, 0, 0));

    if (not new_chunks)
    {
        while (true)
        {
        }
    } // TODO: raise exception instead of hang.

    chunks = new_chunks;
    n_chunks = new_size;
}


// TODO: do we need to propagate using indices in all returns instead of pointers to handle any non-atomic behaviour here?
void* art_alloc(const size_t size_bytes, int flags)
{
    // TODO: process flags to either adjust "size_bytes" or to pass on to "get_suitable_chunk"
    const size_t chunk_idx = get_suitable_chunk(size_bytes); // get suitable chunk may also split in future for alignment reasons
    // if (not chunk) return nullptr;
    if (chunks[chunk_idx].size - size_bytes <= MIN_CHUNK_SIZE)
    {
        chunks[chunk_idx].memory_free = false;
        return chunks[chunk_idx].start;
    }

    // otherwise data doesn't fill chunk enough.
    split_chunk(chunk_idx, size_bytes);

    return chunks[chunk_idx].start;
}


void art_free(const void* ptr)
{
    size_t idx = 0;
    while (idx < n_chunks)
    {
        if (chunks[idx].start == ptr)
        {
            // chunk_t* chunk = &chunks[idx];
            chunks[idx].memory_free = true;
            if (chunks[chunks[idx].prev].memory_free)
            {
                idx = chunks[idx].prev; // swap to prev because current will be deleted
                merge_chunk(idx, chunks[idx].next);
            }
            if (chunks[chunks[idx].next].memory_free)
            {
                merge_chunk(idx, chunks[idx].next);
            }
            // just collect all whole free pages
            if (chunks[idx].size < page_alignment) return;

            auto working_addr = reinterpret_cast<uintptr_t>(chunks[idx].start);
            const uintptr_t end_addr = working_addr + chunks[idx].size;
            working_addr = (working_addr + (page_alignment - 1)) >> base_address_shift << base_address_shift; // need to verify
            const uintptr_t first_boundary = working_addr;
            if (const size_t n_whole_pages = (end_addr - working_addr) / page_alignment; n_whole_pages > 0)
            {
                munmap(reinterpret_cast<void*>(first_boundary), n_whole_pages * page_alignment);
            }
            return;
        }
        ++idx;
    }
}
