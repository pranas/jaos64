BIN = ../bin
ASM = nasm -f bin

floppy.img: stage1 stage2 pad
	cat $(BIN)/stage1 $(BIN)/stage2 $(BIN)/pad > $(BIN)/floppy.img

stage1: stage1.asm
	$(ASM) $< -o $(BIN)/$@
   
stage2: stage2.asm
	$(ASM) $< -o $(BIN)/$@

pad: stage1 stage2
	./pad.sh

clean:
	rm $(BIN)/stage1 $(BIN)/stage2 $(BIN)/pad $(BIN)/floppy.img
