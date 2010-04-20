OUTPUT_FORMAT(elf64-x86-64)
ENTRY(kernel_entry)
SECTIONS
{
    . = 0xc0000000;

    .text :
    {
        *(.text)
        *(.rodata*)
    }

   .data :
   {
        *(.data)
   }

   .ehframe :
   {
       *(.ehframe)
   }

   .bss :   {
       *(.bss)
   }
}
