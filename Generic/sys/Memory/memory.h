//
// Created by artypoole on 13/07/24.
//

#ifndef MEMORY_H
#define MEMORY_H


#include "types.h"

// http://wiki.osdev.org/Memory_Map_(x86)
// "Use the BIOS function INT 15h, EAX=0xE820 to get a reliable map of Extended Memory."
// http://wiki.osdev.org/Detecting_Memory_(x86)#BIOS_Function:_INT_0x15,_EAX_=_0xE820
#ifdef __cplusplus
extern "C" {
#endif

struct multiboot2_tag_mmap; // forward dec - multiboot2.h
extern unsigned char* kernel_brk; // TODO: needed by paging.cpp. Better practice?

// implemented in
void* sbrk(long increment);

// implemented in Specific/<arc>/memory/<paging or similar>.cpp
void mmap_init(struct multiboot2_tag_mmap* mmap);

void* mmap(uintptr_t addr, size_t length, int prot, int flags, int fd, size_t offset);
int munmap(void* addr, size_t length);

void paging_identity_map(uintptr_t phys_addr, size_t size, bool writable, bool user);

#ifdef __cplusplus
}
#endif
#endif //MEMORY_H
