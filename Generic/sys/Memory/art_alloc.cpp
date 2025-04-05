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

#include <logging.h>
#include <stdlib.h>
#include <doomgeneric/p_local.h>

#include "LinkedList.h"
#include "memory.h"


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
    size_t highest_used_chunk = 0;

    size_t idx_from_ptr(const chunk_t* chunk)
    {
        return chunk - chunks;
    }


    bool expand_chunk_array()
    {
        size_t new_size = n_chunks * 2;
        const size_t n_pages = (new_size * sizeof(chunk_t) + page_alignment - 1) / page_alignment;
        new_size = n_pages * page_alignment / sizeof(chunk_t);

        auto* new_chunks = static_cast<chunk_t*>(mmap(0, n_pages * page_alignment, 0, 0, 0, 0));

        if (not new_chunks) return false;

        memcpy(new_chunks, chunks, sizeof(chunk_t) * n_chunks);
        // NOTE: always a full page so munmap is safe.
        munmap(chunks, sizeof(chunk_t) * n_chunks);
        for (size_t i = n_chunks; i < new_size; i++)
        {
            new_chunks[i].array_free = true;
        }
        chunks = new_chunks;
        n_chunks = new_size;
        return true;
    }

    // This is for getting where to store the new chunk info after getting new pages, or when splitting a chunk to assign part of a chunk
    size_t get_next_free_array_entry(const size_t start_idx)
    {
        size_t idx = start_idx;
        while (true)
        {
            while (idx < n_chunks)
            {
                if (chunks[idx].array_free)
                {
                    next_free_array_entry = idx;
                    return idx;
                }

                ++idx;
            }
            expand_chunk_array(); // Expand the array and try again
            return idx;
        }
    }

    size_t get_highest_used_chunk()
    {
        size_t idx = 0;

        while (idx < n_chunks)
        {
            if (chunks[idx].array_free)
            {
                if (idx == 0) return 0;
                return idx - 1;
            }
            ++idx;
        }
        return 0;
    }

    // frees a chunk in the array
    void remove_chunk_from_array(size_t idx)
    {
        if (idx < next_free_array_entry) next_free_array_entry = idx;
        if (idx == end_chunk_idx & end_chunk_idx >= 1) end_chunk_idx = chunks[idx].prev;
        if (idx == highest_used_chunk & highest_used_chunk >= 1) highest_used_chunk = get_highest_used_chunk();
        chunks[idx] = chunk_t{nullptr, 0, 0, 0, false, true};
    }


    // Just stores a new chunk in next available array idx. Returns the index used to store this data
    size_t append_chunk(void* start, size_t size)
    {
        // TODO: update next of the prev chunk on append
        // NORMAL BEHAVIOUR:

        const size_t idx = get_next_free_array_entry(next_free_array_entry);
        chunks[end_chunk_idx].next = idx; // end chunk idx should be "last assigned"
        const size_t new_next = get_next_free_array_entry(idx);
        const chunk_t chunk_data = {start, size, end_chunk_idx, new_next, true, false};
        chunks[idx] = chunk_data;
        next_free_array_entry = new_next;
        // this assumes pages aren't given back so some checks about highest memory loc or something?
        end_chunk_idx = idx;
        highest_used_chunk = idx;
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
        if (chunk_t* end_chunk = &chunks[end_chunk_idx]; end_chunk->start + end_chunk->size == ptr && end_chunk->memory_free) // is it contiguous
        {
            end_chunk->size += got_bytes;
            return end_chunk_idx;
        }
        // assumes always adding to end:
        const size_t chunk_idx = append_chunk(ptr, got_bytes);


        return chunk_idx;
    }

    // This gets the next chunk that's big enough or makes a new one
    size_t get_suitable_chunk(const size_t size_bytes)
    {
        size_t idx = 0;
        while (idx <= highest_used_chunk)
        {
            if (chunks[idx].memory_free & chunks[idx].size >= size_bytes)
            {
                return idx;
            }
            ++idx;
        }

        return get_pages_for_new_chunk(size_bytes);
    }

    void split_chunk(const size_t prior, const size_t size_bytes)
    {
        // TODO: should probably do some rounding or enforcement of minimum 4byte boundaries here.
        void* const new_start = chunks[prior].start + size_bytes;
        const size_t new_size = chunks[prior].size - size_bytes;
        const auto latter = append_chunk(new_start, new_size);
        chunks[latter].next = chunks[prior].next; // doesn't work if not set >.>
        chunks[prior].next = latter;
        chunks[prior].memory_free = false;
        chunks[prior].size = size_bytes;
        chunks[latter].prev = prior;
        chunks[latter].memory_free = true;
    }

    void merge_chunk(const size_t prior, const size_t latter)
    {
        chunks[prior].next = chunks[latter].next;
        chunks[prior].size += chunks[latter].size;
        remove_chunk_from_array(latter);
    }
}

using namespace art_allocator;


/** Initialises memory area by fetching a single page to be used to store chunk info.
 */
void art_memory_init()
{
    LOG("Initialising memory allocator");
    constexpr size_t n_pages = 1;
    constexpr size_t new_size = page_alignment / sizeof(chunk_t);
    auto* new_chunks = static_cast<chunk_t*>(mmap(0, n_pages * page_alignment, 0, 0, 0, 0));

    if (not new_chunks)
    {
        while (true)
        {
        }
    } // TODO: raise exception instead of hang.

    chunks = new_chunks;
    n_chunks = new_size;
    chunks[0] = chunk_t{nullptr, 0, 0, 0, false, true};
    for (size_t i = 0; i < n_chunks; ++i)
    {
        chunks[i].array_free = true;
    }
}


// TODO: do we need to propagate using indices in all returns instead of pointers to handle any non-atomic behaviour here?
void* art_alloc(const size_t size_bytes, int flags)
{
    // TODO: process flags to either adjust "size_bytes" or to pass on to "get_suitable_chunk"
    const size_t chunk_idx = get_suitable_chunk(size_bytes); // get suitable chunk may also split in future for alignment reasons
    chunks[chunk_idx].memory_free = false;
    // if (not chunk) return nullptr;
    if (chunks[chunk_idx].size - size_bytes <= MIN_CHUNK_SIZE)
    {
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
            if (chunks[chunks[idx].prev].memory_free & idx != 0)
            {
                idx = chunks[idx].prev; // swap to prev because current will be deleted
                merge_chunk(idx, chunks[idx].next);
            }
            if (chunks[chunks[idx].next].memory_free)
            {
                merge_chunk(idx, chunks[idx].next);
            }
            // if there are not free whole pages, we are done.
            if (
                chunks[idx].size < page_alignment or
                not(
                    reinterpret_cast<uintptr_t>(chunks[idx].start) % page_alignment == 0 and
                    chunks[idx].size % 4096 == 0
                )
            )
            {
                return;
            }

            // otherwise return those whole pages
            munmap(chunks[idx].start, chunks[idx].size);
            remove_chunk_from_array(idx);
            return;
        }
        ++idx;
    }
}
