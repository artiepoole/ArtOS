/* The bootloader will look at this image and start execution at the symbol
   designated as the entry point. */
ENTRY(_start)

OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386:i386)
/* Tell where the various sections of the object files will be put in the final
   kernel image. */


SECTIONS
{
    . = 0x00100000;
	/* The kernel will live at 3GB + 1MB in the virtual address space, */
	/* which will be mapped to 1MB in the physical address space. */
	/* Note that we page-align the sections. */

    kernel_start = .;
        .multiboot.data : {
            *(.multiboot.data)
        }

        .multiboot.text : AT(ADDR(.multiboot.text))
        {
        KEEP(*(.multiboot.text))
        }

	. += 0xC0000000;
	/* Add a symbol that indicates the start address of the kernel. */
	.text ALIGN (4K) : AT (ADDR (.text) - 0xC0000000)
	{
		*(.text)
	}
	.rodata ALIGN (4K) : AT (ADDR (.rodata) - 0xC0000000)
	{
		*(.rodata)
	}
	.data ALIGN (4K) : AT (ADDR (.data) - 0xC0000000)
	{
		*(.data)
	}
	.bss ALIGN (4K) : AT (ADDR (.bss) - 0xC0000000)
	{
		*(COMMON)
		*(.bss)
		*(.bootstrap_stack)
	}
    /DISCARD/ : { *(.fini_array*) *(.comment) }
	/* Add a symbol that indicates the end address of the kernel. */
	kernel_end = .;
}
