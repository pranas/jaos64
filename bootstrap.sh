nasm -f bin bootloader.s -o bootloader.bin
nasm -f bin bootloader2.s -o bootloader2.bin
cat bootloader.bin bootloader2.bin pad2 > floppy.img
