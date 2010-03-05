BOOT = boot
BIN = bin
ASM = nasm -f bin

all:
	$(MAKE) -C boot/

clean:
	$(MAKE) clean -C boot/

run:
	bochs -q -rc dbgrc -f bochsrc