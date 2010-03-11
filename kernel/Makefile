BIN = ../bin
INCLUDE_PATH = ../include
LIBRARY_PATH = ../lib
CC = gcc
CFLAGS = -ffreestanding -mcmodel=large -nostdlib -m64 -I $(INCLUDE_PATH) -L $(LIBRARY_PATH)
LD = ld
LDFLAGS = -nostdlib -nodefaultlibs -m elf_x86_64 -T link.ld

all: kernel

kernel: kernel.o
	$(LD) $(LDFLAGS) $^ -o $(BIN)/$@
	rm kernel.o

kernel.o: main.c
	$(CC) -c $(CFLAGS) $^ -o $@

clean:
	-rm $(BIN)/kernel
