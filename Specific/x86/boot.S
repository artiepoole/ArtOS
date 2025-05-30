
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
    # first page: 0x00100000 is the 256th page. so page dir = 1, page table entry = 256, which is first page table which points to 0x00100000
    # TODO: we need to map the first page from 0x10000 in lower half before the jump

    # updates page directory to know first page table exists
    movl $(boot_page_tables - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 0

    # get page table entry physical addr
	movl $(boot_page_tables - 0xC0000000 + 4*256), %edi
	orl $0x3, %edi

	# create page table entry to identity map this one page which contains this mapping code
	movl 0x00100000, %edi
	orl $0x3, %edi
    # get address of page table entry
    movl $(boot_page_tables - 0xC0000000 + 4*256), %edx
	# write it into the page table
	movl %edi, (%edx)

# We want to loop from 0 to 256 to update the 256 entries of the page directory to point to the start of each page table
# We then want to loop from 0 to 1024*256 to write to the page tables
# page directory address is boot_page_directory + (index + 768) * 4
# Page table entry physical address is boot_page_tables - 0xC0000000 + index * 4
# so at  boot_page_directory + index * 4 we want to write (boot_page_tables - 0xC0000000 + index * 4096  or 0x3)
# ecx is idx so we will loop from 256 to 0

populate_page_directory_lower:
    movl $0, %ecx
    # load the start address of the destination (end of the directory) within the boot page directory into esi
    movl $(boot_page_directory - 0xC0000000), %esi
    movl $(boot_page_tables - 0xC0000000), %edi
    page_dir_loop_low:
        # This copies the page table address and sets the present and writable flags into edx then writes the adddress to the page directory.
        movl %edi, %edx
        orl $0x3, %edx
        movl %edx, (%esi)

        # increment the index of the page directory
        addl $4, %esi
        # increment the address of the table
        addl $4096, %edi
        # increment index
        incl %ecx
        cmpl $256, %ecx
        jl page_dir_loop_low

# were going to count up this time
populate_page_tables_lower:
    # ecx stores the counter
    movl $256, %ecx
    # edi stores the physical address of the destination memory region
    movl $0, %edi
    # esi stores the page table entry address
    movl $(boot_page_tables - 0xC0000000), %esi

    page_table_loop_low:

        # copies the dest. physical address and then adds the flags for present and writable
        movl %edi, %edx
        orl $0x3, %edx
        # store the physical address in the page table
        movl %edx, (%esi)

        # increment the physical address of the page table entry to write to
        addl $4, %esi
        # increment the reuslting physical address
        addl $4096, %edi
        incl %ecx
        cmpl $(4096*256), %ecx
        jl page_table_loop_low


populate_page_directory_upper:
    movl $768, %ecx
    # load the start address of the destination (end of the directory) within the boot page directory into esi
    movl $(boot_page_directory - 0xC0000000 + (768 * 4)), %esi
    movl $(boot_page_tables - 0xC0000000 + (4096 * 768)), %edi
    page_dir_loop_high:
        # This copies the page table address and sets the present and writable flags into edx then writes the adddress to the page directory.
        movl %edi, %edx
        orl $0x3, %edx
        movl %edx, (%esi)

        # increment the index of the page directory
        addl $4, %esi
        # increment the address of the table
        addl $4096, %edi
        # increment index
        incl %ecx
        cmpl $1024, %ecx
        jl page_dir_loop_high

##### this logic is wrong! It should map from 0x100000 -> 1GB to be accessed from 0xc0100000 -> 0x1000000000
# were going to count up this time. We want to map from 768*2096 + 256 -> 1024*4096 addresses to point to addresses from 256 to 256*4096
populate_page_tables_upper:
    # ecx stores the counter
    movl $256, %ecx
    # edi stores the physical address of the destination memory region
    movl $0x100000, %edi
    # esi stores the page table entry address
    movl $(boot_page_tables - 0xC0000000 + (4096*768) + (256*4)), %esi

    page_table_loop_high:

        # copies the dest. physical address and then adds the flags for present and writable
        movl %edi, %edx
        orl $0x3, %edx
        # store the physical address in the page table
        movl %edx, (%esi)

        # increment the physical address of the page table entry to write to
        addl $4, %esi
        # increment the reuslting physical address
        addl $4096, %edi
        incl %ecx
        cmpl $(4096*256), %ecx
        jl page_table_loop_high

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






