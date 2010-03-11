BOOT = boot
BIN = bin
ASM = nasm -f bin

all:
	$(MAKE) -C boot/
	$(MAKE) -C kernel/
	$(MAKE) -C bin/

clean:
	$(MAKE) clean -C boot/
	$(MAKE) clean -C kernel/
	$(MAKE) clean -C bin/

run:
	bochs -q -rc dbgrc -f bochsrc
