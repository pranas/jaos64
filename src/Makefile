BOOT = boot
BIN = bin
ASM = nasm -f bin

floppy.img: stage1 stage2 pad
	cat stage1 stage2 pad > ../$(BOOT)/floppy.img

stage1: stage1.s
	$(ASM) $< -o $@
   
stage2: stage2.s
	$(ASM) $< -o $@

pad: stage1 stage2
	./pad.sh

clean:
	rm stage1 stage2 pad
