ENTRY(main)

OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386:i386)


SECTIONS
{
   . = 1M;


    .text BLOCK(4K) : ALIGN(4K)
    {
        *(.text*)
        *(.rodata)
    }

	/* Read-write data (initialized) */
	/*.data */
    .data BLOCK(4K) : ALIGN(4K)
    {
        start_ctors = .;
        KEEP(*( .init_array ));
        KEEP(*(SORT_BY_INIT_PRIORITY( .init_array.* )));
        end_ctors = .;
        *(.data)
        *(.rawdata*)
    }

	/* Read-write data (uninitialized) and stack */
	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(.bss)
	}

    /DISCARD/ : { *(.fini_array*) *(.comment) }

}