ENTRY(main32) /* begin execution @ main32, which is in start.asm */
OUTPUT_FORMAT(binary) /* everything is binary, so no distinction between code & data (KISS) */
SECTIONS
{
    . = 0x8000; /* kernel origin */

    .text : ALIGN(4096) /*code section (note that C code is 16-byte aligned, so assembly code must also be) */
    {
        *(.text)
    }

    .asm : ALIGN(4096) /* unaligned assembly code goes here */
    {
        *(.asm)
    }

    .rodata : ALIGN(4096) /* readonly data section */
    {
        *(.rodata)
    }

    .data : ALIGN(4096) /* data section */
    {
        *(.data)
    }

    .bss : ALIGN(4096) /* static data section */
    {
        *(COMMON)
        *(.bss)
    }
}
