
/* Declare Constants for the multiboot header. */
#define ASM_FILE 1
#include <multiboot2.h>

.set WIDTH, TARGET_WIDTH  /* requested width */
.set HEIGHT, TARGET_HEIGHT /* requested height */
.set DEPTH, 32    /* requested bits per pixel BPP */


.section .multiboot.data, "aw"
/*  Align 64 bits boundary. */
.align  8
/*  Multiboot header. */
.globl multiboot_header
multiboot_header:
            /*  magic */
    .long   MULTIBOOT2_HEADER_MAGIC
    /*  ISA: i386 */
    .long   MULTIBOOT2_ARCHITECTURE_I386
    /*  Header length. */
    .long   multiboot_header_end - multiboot_header
    /*  checksum */
    .long   -(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT2_ARCHITECTURE_I386 + (multiboot_header_end - multiboot_header))

    .align 8
    multiboot_requests_start:
        .short MULTIBOOT2_HEADER_TAG_INFORMATION_REQUEST
        .short 0
        .long multiboot_requests_end-multiboot_requests_start
        .long MULTIBOOT2_TAG_TYPE_BOOTDEV
        .long MULTIBOOT2_HEADER_TAG_FRAMEBUFFER
        .long MULTIBOOT2_TAG_TYPE_EFI32
        .long MULTIBOOT2_TAG_TYPE_ACPI_NEW
        .long MULTIBOOT2_TAG_TYPE_ACPI_OLD
    multiboot_requests_end:

    .align 8
    framebuffer_tag_start:
        .short MULTIBOOT2_HEADER_TAG_FRAMEBUFFER
        .short MULTIBOOT2_HEADER_TAG_OPTIONAL
        .long framebuffer_tag_end - framebuffer_tag_start
        .long WIDTH
        .long HEIGHT
        .long DEPTH
    framebuffer_tag_end:

    .align  8
        .short MULTIBOOT2_HEADER_TAG_END
        .short 0
        .long 8

multiboot_header_end:


.section .bootstrap_stack, "aw", @nobits
.globl kernel_stack_top, kernel_interrupt_stack_top, kernel_stack_bottom
.section .bss
.align 16
kernel_stack_bottom:
.skip 1024*1024 # 1MB
kernel_interrupt_stack_top:
.skip 1024*1024 # 1MB
kernel_stack_top:


# Preallocate pages used for paging. Don't hard-code addresses and assume they
# are available, as the bootloader might have loaded its multiboot structures or
# modules there. This lets the bootloader know it must avoid the addresses.

.section .bss, "aw", @nobits
	.align 4096
.globl boot_page_directory,  boot_page_tables
boot_page_directory:
	.skip 4096
boot_page_tables:
	.skip 4096*1024 # entire memory region available

.section .multiboot.text, "a"
.globl  start, _start
.type _start, @function
start:
_start:

    # Physical address of boot_page_tables.
	movl $(boot_page_tables - 0xC0000000), %edi
	# First address to map is address 0.
	movl $0, %esi
    # is supposed to map 1024 entries for 256 page tables (only the top 1GB is mapped here)
	movl $(1024*256), %ecx

start_mapping:
    # Starts at 0 memory and loops until end of kernel. It does the mapping (in 1:) and then incrementing (in 2:) until kernel end is reached then it does 3:
	cmpl 0x00100000, %esi
	jl increment_pointers
	cmpl $(0xFFFFFFFF - 0xC0000000), %esi
	jge do_mapping

	# Map physical address as "present, writable". Note that this maps
	# .text and .rodata as writable. Mind security and map them as non-writable.
	movl %esi, %edx
	orl $0x003, %edx
	movl %edx, (%edi)

increment_pointers:
	# Size of page is 4096 bytes.
	addl $4096, %esi
	# Size of entries in boot_page_table1 is 4 bytes.
	addl $4, %edi
	# Loop to the next entry if we haven't finished.
	loop start_mapping

do_mapping:

    # Map the bootstrap page table to both virtual addresses 0x00000000 and 0xC0000000.
    movl $(boot_page_tables - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 0

    # %esi contains how many bytes we mapped
    # Calculate (mapped_bytes + 4MB-1) / 4MB, result stays in %esi
    addl $(4 * 1024 * 1024 - 1), %esi
    shrl $22, %esi             # %esi now holds number of 4MB chunks to map

    xorl %ecx, %ecx            # %ecx = loop index (i)

map_upper_tables:
    cmpl %esi, %ecx
    jge done_mapping_upper

    # Compute physical address of page table: boot_page_tables + ecx * 4096
    movl %ecx, %edx
    shl $12, %edx                              # edx = ecx * 4096
    movl $(boot_page_tables - 0xC0000000), %edi
    addl %edx, %edi                            # edi = physical address of page table
    orl $0x003, %edi                           # mark present + writable

    # Compute offset in page directory: (768 + ecx) * 4 -> edx
    movl %ecx, %edx
    addl $768, %edx
    shl $2, %edx

    # Write page table address into the directory
    movl %edi, boot_page_directory - 0xC0000000(, %edx, 1)

    incl %ecx
    jmp map_upper_tables

done_mapping_upper:
map_mutliboot_table:
        # Assume %ebx holds physical address to map; copy it to %esi (safe register)
    movl %ebx, %esi             # %esi = physical address to map

    # Calculate page directory index (bits 22-31) in %edi
    movl %esi, %edi
    shrl $22, %edi              # %edi = page directory index

    # Calculate page table index (bits 12-21) in %edx
    movl %esi, %edx
    shrl $12, %edx              # shift right 12
    andl $0x3FF, %edx           # mask lower 10 bits -> page table index

    # Calculate address of page table: boot_page_tables + %edi * 4096
    leal boot_page_tables - 0xC0000000, %ecx # temporarily use %ecx for calculation
    shll $12, %edi              # %edi = %edi * 4096 (page table offset)
    addl %edi, %ecx             # %ecx = &boot_page_tables + offset

    # Calculate page table entry address: %eax + %edx * 4
    movl %edx, %edi
    shll $2, %edi               # %edi = page table index * 4 (byte offset)
    addl %edi, %ecx             # %eax = address of page table entry

    # Prepare page table entry value in %esi
    andl $0xFFFFF000, %esi      # page align address
    orl $0x003, %esi            # flags: present + writable

    # Store the page table entry
    movl %esi, (%ecx)

done_mapping:
    # Set cr3 to the address of the boot_page_directory.
    movl $(boot_page_directory - 0xC0000000), %ecx
    movl %ecx, %cr3

    # Enable paging and the write-protect bit.
    movl %cr0, %ecx
    orl $0x80010000, %ecx
    movl %ecx, %cr0

    # Jump to higher half with an absolute jump.
    lea 4f, %ecx
    jmp *%ecx

.section .text

4:
	# At this point, paging is fully set up and enabled.

	# Unmap the identity mapping as it is now unnecessary.
	movl $0, boot_page_directory + 0

	# Reload crc3 to force a TLB flush so the changes to take effect.
	movl %cr3, %ecx
	movl %ecx, %cr3

	# Set up the stack.
	mov $kernel_stack_top, %esp

    pushl   $0
    popf

    /*  Push the pointer to the Multiboot information structure. */
    push %ebx
    /*  Push the magic value. */
    push %eax

	# Enter the high-level kernel.
	call kernel_main

	# Infinite loop if the system has nothing more to do.
	cli
1:	hlt
	jmp 1b






