BOOT = boot
BIN = bin
ASM = nasm -f bin

all:
	$(MAKE) -C src/

clean:
	$(MAKE) clean -C src/
