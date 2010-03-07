BIN = ../bin
INCLUDE_PATH = ../include
LIBRARY_PATH = ../lib
GCC = gcc -ffreestanding -mcmodel=large -nostdlib -m64 -I $(INCLUDE_PATH) -L $(LIBRARY_PATH) -c -o
LD = ld -nostdlib -nodefaultlibs -m elf_x86_64 -T link.ld -o

kernel: kernel.o
	$(LD) $(BIN)/kernel $(BIN)/kernel.o

kernel.o:
	$(GCC) $(BIN)/kernel.o main.c

clean:
	rm $(BIN)/kernel.o $(BIN)/kernel