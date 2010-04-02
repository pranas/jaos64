BIN = ../bin
VPATH = .
INCLUDE_PATH = ../include
LIBRARY_PATH = ../lib
CC = gcc
LD = ld
CFLAGS = -ffreestanding -mcmodel=large -nostdlib -m64 \
		 -I $(INCLUDE_PATH) -L $(LIBRARY_PATH)
LDFLAGS = -nostdlib -nodefaultlibs -m elf_x86_64 -T link.ld

KERNELNAME = kernel
KERNEL = $(addprefix $(BIN)/, $(KERNELNAME))
OBJS = *.o
AS = nasm

all: $(KERNEL)

$(KERNEL): kernel.o monitor.o io.o gdt.o gdt_flush.o link.ld
	$(LD) $(LDFLAGS) $^ -o $@
	-rm $(OBJS)

kernel.o: main.c
	$(CC) -c $(CFLAGS) $< -o $@

gdt.o: gdt.c
	$(CC) -c $(CFLAGS) $< -o $@

monitor.o: monitor.c
	$(CC) -c $(CFLAGS) $< -o $@

io.o: io.c
	$(CC) -c $(CFLAGS) $< -o $@

gdt_flush.o: gdt_flush.asm
	$(AS) -f elf64 -o $@ $< 

clean:
	-rm $(KERNEL) $(OBJS)
