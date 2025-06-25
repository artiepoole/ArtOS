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
	    kernel_start = .;
        *(.text*)
        *(.rodata*)
        *(.eh_frame*)
        *(.eh_frame_hdr*)
	}
	.data ALIGN (4K) : AT (ADDR (.data) - 0xC0000000)
	{
        *(.data*)
        *(.rawdata*)
	}

	.init_array ALIGN(4k) : AT(ADDR(.init_array) - 0xC0000000)
    {
        start_ctors = .;
        KEEP(*(.init_array))
        KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*)))
        end_ctors = .;
    }
	.bss ALIGN (4K) : AT (ADDR (.bss) - 0xC0000000)
	{
		*(COMMON)
		*(.bss)
		*(.bootstrap_stack)
	}

	.trampoline.text ALIGN (4K) : AT (ADDR (.trampoline.text) - 0xC0000000){
        kernel_trampoline_text_start = .;
        *(.trampoline.text)
        kernel_trampoline_text_end = .;
    }

    .trampoline.data ALIGN(4K) :  AT (ADDR (.trampoline.data) - 0xC0000000) {
        kernel_trampoline_data_start = .;
        *(.trampoline.data)
        *(.trampoline.rodata)
        kernel_trampoline_data_end = .;
    }

    /DISCARD/ : { *(.fini_array*) *(.comment) }
	/* Add a symbol that indicates the end address of the kernel. */
	kernel_end = .;
}
