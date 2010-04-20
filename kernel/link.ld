OUTPUT_FORMAT(elf64-x86-64)
ENTRY(kernel_entry)
SECTIONS
{
    . = 0x00000000c0000000;

    .text :
    {
        *(.text)
        *(.rodata*)
    }

   .data :
   {
        *(.data)
   }

   .bss :   {
       *(.bss)
   }
}
