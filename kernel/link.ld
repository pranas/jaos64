OUTPUT_FORMAT(elf64-x86-64)
ENTRY(kernel_entry)
SECTIONS
{
    . = 0xc000000000000000;

    .text : AT(ADDR(.text) - 0xc000000000000000)
    {
        _code = .;
        *(.text)
        *(.rodata*)
        . = ALIGN(4096);
    }

   .data : AT(ADDR(.data) - 0xc000000000000000)
   {
        _data = .;
        *(.data)
        . = ALIGN(4096);
   }

   .ehframe : AT(ADDR(.ehframe) - 0xc000000000000000)
   {
       _ehframe = .;
       *(.ehframe)
        . = ALIGN(4096);
   }

   .bss : AT(ADDR(.bss) - 0xc000000000000000)
   {
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