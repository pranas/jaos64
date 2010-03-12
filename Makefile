BOOT = boot
BIN = bin
ASM = nasm -f bin

all:
	$(MAKE) -C boot/
#	$(MAKE) -C kernel/

clean:
	$(MAKE) clean -C boot/
	$(MAKE) clean -C kernel/

run:
	bochs -q -rc dbgrc -f bochsrc