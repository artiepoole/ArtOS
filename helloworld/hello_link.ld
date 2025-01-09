ENTRY(main)
SECTIONS
{
    . = 0x100000; /* Base load address */
    .text : { *(.text) }
    .data : { *(.data) }
    .bss : { *(.bss) }
}