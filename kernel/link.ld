OUTPUT_FORMAT(elf64-x86-64)
ENTRY(kernel_entry)
SECTIONS
{
    . = 0x6000;

    .text :
    {
        _code = .;
        *(.text)
        *(.rodata*)
        . = ALIGN(4096);
    }

   .data :
   {
        _data = .;
        *(.data)
        . = ALIGN(4096);
   }

   .ehframe :
   {
       _ehframe = .;
       *(.ehframe)
        . = ALIGN(4096);
   }

   .bss :   {
       _bss = .;
       *(.bss)
       . = ALIGN(4096);
   }

   _end = .;

   /DISCARD/ :
   {
        *(.comment)
   }
}
