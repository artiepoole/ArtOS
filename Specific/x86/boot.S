
/* Declare Constants for the multiboot header. */
#define ASM_FILE 1
#include <multiboot2.h>



.set WIDTH, TARGET_WIDTH  /* requested width */
.set HEIGHT, TARGET_HEIGHT /* requested height */
.set DEPTH, 32    /* requested bits per pixel BPP */

/*
Declare a multiboot header that marks the program as a kernel. These are magic
values that are documented in the multiboot standard. The bootloader will
search for this signature in the first 8 KiB of the kernel file, aligned at a
32-bit boundary. The signature is in its own section so the header can be
forced to be within the first 8 KiB of the kernel file.

from https://www.gnu.org/software/grub/manual/multiboot/multiboot.html#OS-image-format

0	u32	magic	required
4	u32	flags	required
8	u32	checksum	required
12	u32	header_addr	if flags[16] is set
16	u32	load_addr	if flags[16] is set
20	u32	load_end_addr	if flags[16] is set
24	u32	bss_end_addr	if flags[16] is set
28	u32	entry_addr	if flags[16] is set
32	u32	mode_type	if flags[2] is set
36	u32	width	if flags[2] is set
40	u32	height	if flags[2] is set
44	u32	depth	if flags[2] is set
*/
/*
*.section .multiboot
*    .align 4
*    .long MAGIC
*    .long FLAGS
*    .long CHECKSUM
*    .long HEADER_ADDR
*    .long LOAD_ADDR
*    .long LOAD_END_ADDR
*    .long BSS_END_ADDR
*    .long ENTRY_ADDR
*    .long MODE_TYPE
*    .long WIDTH
*    .long HEIGHT
*    .long DEPTH
*    /* enough space for the returned header */
/*    .space 4 * 13
*/



/*
.section .multiboot2
    .align 4
    .long MAGIC
    .long ARCHITECTURE
    .long HEADER_LENGTH
    .long CHECKSUM
    .long TAGS
*/

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


.section .bss
.align 16
stack_bottom:
.skip 256*1024*1024 # 200MiB
stack_top:

.section .text
.globl  start, _start
start:
_start:
	/*
	The bootloader has loaded us into 32-bit protected mode on a x86
	machine. Interrupts are disabled. Paging is disabled. The processor
	state is as defined in the multiboot standard. The kernel has full
	control of the CPU. The kernel can only make use of hardware features
	and any code it provides as part of itself. There's no printf
	function, unless the kernel provides its own <stdio.h> header and a
	printf implementation. There are no security restrictions, no
	safeguards, no debugging mechanisms, only what the kernel provides
	itself. It has absolute and complete power over the
	machine.
	*/

	/*
	To set up a stack, we set the esp register to point to the top of the
	stack (as it grows downwards on x86 systems). This is necessarily done
	in assembly as languages such as C cannot function without a stack.
	*/

	/*
	This is a good place to initialize crucial processor state before the
	high-level kernel is entered. It's best to minimize the early
	environment where crucial features are offline. Note that the
	processor is not fully initialized yet: Features such as floating
	point instructions and instruction set extensions are not initialized
	yet. The GDT should be loaded here. Paging should be enabled here.
	C++ features such as global constructors and exceptions will require
	runtime support to work as well.
	*/

	/*
	Enter the high-level kernel. The ABI requires the stack is 16-byte
	aligned at the time of the call instruction (which afterwards pushes
	the return pointer of size 4 bytes). The stack was originally 16-byte
	aligned above and we've pushed a multiple of 16 bytes to the
	stack since (pushed 0 bytes so far), so the alignment has thus been
	preserved and the call is well defined.
	*/
    mov $stack_top, %esp

    /* Reset flags*/

    pushl   $0
    popf

    /*  Push the pointer to the Multiboot information structure. */
    push %ebx
    /*  Push the magic value. */
    push %eax

    /* push %ecx Not necessary now. */
    call kernel_main
	/*
	If the system has nothing more to do, put the computer into an
	infinite loop. To do that:
	1) Disable interrupts with cli (clear interrupt enable in eflags).
	   They are already disabled by the bootloader, so this is not needed.
	   Mind that you might later enable interrupts and return from
	   kernel_main (which is sort of nonsensical to do).
	2) Wait for the next interrupt to arrive with hlt (halt instruction).
	   Since they are disabled, this will lock up the computer.
	3) Jump to the hlt instruction if it ever wakes up due to a
	   non-maskable interrupt occurring or due to system management mode.
	*/
	cli
	subl $4, %esp
    movl $0, %esp

    call __cxa_finalize

    addl $4, %esp

loop:
    hlt
    jmp     loop




/*
Set the size of the _start symbol to the current location '.' minus its start.
This is useful when debugging or when you implement call tracing.
*/
.size _start, . - _start
.size start, . - start
