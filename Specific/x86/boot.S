
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
	.skip 4096*256

.section .multiboot.text, "a"
.globl  start, _start
.type _start, @function
start:
_start:

    # Physical address of boot_page_table1.
	movl $(boot_page_tables - 0xC0000000), %edi
	# First address to map is address 0.
	# TODO: Start at the first kernel page instead. Alternatively map the first
	#       1 MiB as it can be generally useful, and there's no need to
	#       specially map the VGA buffer.
	movl $0, %esi
	# Map 1023 pages. The 1024th will be the VGA text buffer.
	movl $(1024*256), %ecx

1:
	# Only map the kernel.
	cmpl $kernel_start, %esi
	jl 2f
	cmpl $(kernel_end - 0xC0000000), %esi
	jge 3f

	# Map physical address as "present, writable". Note that this maps
	# .text and .rodata as writable. Mind security and map them as non-writable.
	movl %esi, %edx
	orl $0x003, %edx
	movl %edx, (%edi)

2:
	# Size of page is 4096 bytes.
	addl $4096, %esi
	# Size of entries in boot_page_table1 is 4 bytes.
	addl $4, %edi
	# Loop to the next entry if we haven't finished.
	loop 1b

3:
	# Map VGA video memory to 0xC03FF000 as "present, writable".
	# movl $(0x000B8000 | 0x003), boot_page_tables - 0xC0000000 + (4096*1024-1) * 4

	# The page table is used at both page directory entry 0 (virtually from 0x0
	# to 0x3FFFFF) (thus identity mapping the kernel) and page directory entry
	# 768 (virtually from 0xC0000000 to 0xC03FFFFF) (thus mapping it in the
	# higher half). The kernel is identity mapped because enabling paging does
	# not change the next instruction, which continues to be physical. The CPU
	# would instead page fault if there was no identity mapping.

	# Map the page table to both virtual addresses 0x00000000 and 0xC0000000.
	movl $(boot_page_tables - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 0
	movl $(boot_page_tables - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 768 * 4

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






