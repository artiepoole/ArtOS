ENTRY(main)
OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386:i386)

SECTIONS
{
    . = 0x100000; /* Base load address */
    .text : { *(.text) }
    .data : { *(.data) }
    .bss : { *(.bss) }
}