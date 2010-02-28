nasm -f bin bootloader.s -o bootloader.bin
nasm -f bin bootloader2.s -o bootloader2.bin
SIZE1=`du -b bootloader.bin | awk '{print $1 }'`
SIZE2=`du -b bootloader2.bin | awk '{print $1 }'`
COUNT=$(( ((1440 * 1024) - $SIZE1 - $SIZE2) / 512 ))
dd if=/dev/zero of=pad bs=512 count=$COUNT
cat bootloader.bin bootloader2.bin pad > floppy.img
