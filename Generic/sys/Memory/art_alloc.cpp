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
    /**
     * nodes and data for a linked list where the nodes are stored in a free list
     */
    struct chunk_t
    {
        void* start; // address in bytes
        size_t size; // in bytes
        size_t prev;
        size_t next;
        bool memory_free : 1;
        bool array_free : 1;
    };

    chunk_t* chunks; // A "free list" of chunks
    size_t n_chunks = 0; // current size of the chunks free list
    size_t next_free_array_entry = 0; // the smallest idx in the free list which is free to write to
    size_t end_chunk_idx = 0; // the index of the chunk which is the "tail" of the linked list
    size_t highest_used_chunk = 0; // the index of the chunk which has the highest index in the free list (should be equal to n_chunks but might not be?)


    /** Doubles the capacity of the chunks free list
     *
     * @return true for success and false for failure
     */
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
    /** Scans the free list to find a suitable place to store a new chunk. This will call function to expand the free list if full
     *
     * @param start_idx the index at which to look above, saves time
     * @return the
     */
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


    /** Scans the chunks array for the index of the highest used chunk
     *
     * @return the index of the highest used chunk or 0 if none are free.
     */
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


    /** When the chunk is merged or otherwise removed from play, this will free an entry in the free list
     * It also handles updating the tracker items such as next_free_array_entry
     *
     * @param idx the index of the chunk entry to be freed
     */
    void remove_chunk_from_array(const size_t idx)
    {
        if (idx < next_free_array_entry) next_free_array_entry = idx;
        if (idx == end_chunk_idx & end_chunk_idx >= 1) end_chunk_idx = chunks[idx].prev;
        if (idx == highest_used_chunk & highest_used_chunk >= 1) highest_used_chunk = get_highest_used_chunk();
        chunks[idx] = chunk_t{nullptr, 0, 0, 0, false, true};
    }


    /** Creates a new chunk struct and stores it in the chunks free list
     *
     * @param start the start address of the memory region handled by this chunk
     * @param size the size of the aforementioned memory region
     * @return the index of the newly stored chunk
     */
    size_t append_chunk(void* start, const size_t size)
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


    /** Calls mmap to get new memory from the paging system to be used by the calling process.
     *
     * @param size_bytes number of bytes to allocate
     * @param alignment_size the alignment in bytes
     * @return the index associated with the chunk which is now at least size_bytes big (can be merged)
     */
    size_t get_pages_for_new_chunk(const size_t size_bytes, const size_t alignment_size = 1)
    {
        const size_t n_pages = (size_bytes + page_alignment - 1) / page_alignment;
        const size_t got_bytes = n_pages * page_alignment;
        const auto ptr = mmap(0, n_pages * page_alignment, 0, 0, 0, 0);
        if (!ptr)
        {
            exit(-1);
        }
        if (chunk_t* end_chunk = &chunks[end_chunk_idx];
            end_chunk->start + end_chunk->size == ptr &&
            end_chunk->memory_free &&
            reinterpret_cast<uintptr_t>(end_chunk->start) % alignment_size == 0) // is it contiguous and suitable
        {
            end_chunk->size += got_bytes;
            return end_chunk_idx;
        }
        // assumes always adding to end:
        const size_t chunk_idx = append_chunk(ptr, got_bytes);

        return chunk_idx;
    }


    /**
     *
     * @param prior index of the chunk to be split
     * @param size_bytes the amount of bytes to keep in the first chunk
     * @return the index of latter chunk which is created
     */
    size_t split_chunk(const size_t prior, const size_t size_bytes)
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
        return latter;
    }


    /** Merge two chunks which are contiguous in memory
     *
     * @param prior the index of the chunk which is earlier in memory
     * @param latter the index of the chunk which is later in memory
     */
    void merge_chunk(const size_t prior, const size_t latter)
    {
        chunks[prior].next = chunks[latter].next;
        chunks[prior].size += chunks[latter].size;
        remove_chunk_from_array(latter);
    }


    /** Get the next suitable chunk for memory which must be aligned
     *
     * @param size_bytes required number of bytes
     * @param alignment_size alignment in bytes e.g. 4 for 32bit integer
     * @return index of first found/created suitable chunk
     */
    size_t get_aligned_suitable_chunk(const size_t size_bytes, const size_t alignment_size)
    {
        size_t idx = 0;
        while (idx <= highest_used_chunk)
        {
            chunk_t* chunk = &chunks[idx];
            ++idx;
            if (!chunk->memory_free) continue;
            if (chunk->size < size_bytes) continue;
            if (reinterpret_cast<u32>(chunk->start) % alignment_size + size_bytes >= chunk->size) // if whole
            {
                if (reinterpret_cast<u32>(chunk->start) % alignment_size != 0)
                {
                }
                return idx;
            }
        }
        return get_pages_for_new_chunk(size_bytes, alignment_size);
    }

    /** Get the next suitable chunk for memory which doesn't care about alignment
     *
     * @param size_bytes required number of bytes
     * @return index of first found/created suitable chunk
     */
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
        // TODO: this could calculate how many /more/ bytes to get instead of always getting enough for the whole object:
        // i.e. if there are 5000 bytes required, this gets 2 pages but if there are > 4 bytes free at the end of the last
        // chunk then this should only request one page and append the new page to old chunk. This is complicated to do
        // without a refactor because the appending of a chunk also merges if possible atm.
        return get_pages_for_new_chunk(size_bytes);
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


/** Allocate memory for kernel use. Uses entire pages to store chunk information as a hybrid of a linked list with its nodes stored in a "free list" type of array.
 * When the chunk node array is full, that is reallocated by doubling its size. When chunks are allocated, chunks are split down to a minimum size of 64 bytes.
 *
 * @param size_bytes number of bytes to allocate
 * @param alignment_size level of alignment in bytes - defaults to 1 byte aligned (0 acts as 1)
 * @param flags optional flags
 * @return pointer to start of allocated memory in the chunk
 */
void* art_alloc(const size_t size_bytes, size_t alignment_size, int flags)
{
    size_t chunk_idx;
    if (alignment_size == 0 or alignment_size == 1)
    {
        chunk_idx = get_suitable_chunk(size_bytes);
    }
    else
    {
        chunk_idx = get_aligned_suitable_chunk(size_bytes, alignment_size);
    }
    chunks[chunk_idx].memory_free = false;

    // if excess space in chunk is too small to split, return it as is
    if (chunks[chunk_idx].size - size_bytes <= MIN_CHUNK_SIZE)
    {
        return chunks[chunk_idx].start;
    }

    // otherwise data doesn't fill chunk enough so we split it and reserve remaining unused memory.
    split_chunk(chunk_idx, size_bytes);

    return chunks[chunk_idx].start;
}


/** Frees up previously allocated chunks when ptr is a valid start value of a chunk.
 * Upon freeing, previous and next chunks are checked to see if they can be merge with the newly freed chunk
 *
 * @param ptr address of the chunk allocated using art_alloc.
 */
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

            // otherwise unmap those whole pages
            munmap(chunks[idx].start, chunks[idx].size);
            remove_chunk_from_array(idx);
            return;
        }
        ++idx;
    }
}
