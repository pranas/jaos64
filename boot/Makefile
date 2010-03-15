BIN = ../bin
ASM = nasm -f bin

all: stage1 stage2

stage1: stage1.asm
	$(ASM) $< -o $(BIN)/$@
   
stage2: stage2.asm
	$(ASM) $< -o $(BIN)/$@

clean:
	rm $(BIN)/stage1 $(BIN)/stage2
