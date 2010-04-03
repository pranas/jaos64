OUTPUT_FORMAT(elf64-x86-64)
ENTRY(kernel_entry)
SECTIONS
{
    . = 0x6000;

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
