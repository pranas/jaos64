BIN = ../bin
VPATH = .
INCLUDE_PATH = ../include
LIBRARY_PATH = ../lib
CC = gcc
LD = ld
CFLAGS = -ffreestanding -mcmodel=large -nostdlib -m64 \
		 -I$(INCLUDE_PATH) 
LDFLAGS = -L$(LIBRARY_PATH) -nostdlib -nodefaultlibs -m elf_x86_64 -T link.ld -lcorelib --traditional-format

KERNELNAME = kernel
KERNEL = $(addprefix $(BIN)/, $(KERNELNAME))
OBJS = *.o
AS = nasm
ASFLAGS = -f elf64

all: $(KERNEL)

$(KERNEL): kernel.o monitor.o io.o gdt.o idt.o isr.o gdt_flush.o memman.o interrupt.o link.ld
	$(LD) $^ $(LDFLAGS) -o $@
	strip $@
	-rm $(OBJS)

kernel.o: main.c
	$(CC) -c $(CFLAGS) $< -o $@

gdt.o: gdt.c
	$(CC) -c $(CFLAGS) $< -o $@

monitor.o: monitor.c
	$(CC) -c $(CFLAGS) $< -o $@

io.o: io.c
	$(CC) -c $(CFLAGS) $< -o $@

isr.o: isr.c
	$(CC) -c $(CFLAGS) $< -o $@

idt.o: idt.c
	$(CC) -c $(CFLAGS) $< -o $@
	
memman.o: memman.c
	$(CC) -c $(CFLAGS) $< -o $@

gdt_flush.o: gdt_flush.asm
	$(AS) $(ASFLAGS) -o $@ $< 
interrupt.o: interrupt.asm
	$(AS) $(ASFLAGS) -o $@ $< 

clean:
	-rm $(KERNEL) $(OBJS) defaultlibs
