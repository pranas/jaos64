BIN = ../bin
VPATH = .
INCLUDE_PATH = ../include
LIBRARY_PATH = ../lib
CC = gcc
LD = ld
CFLAGS = -ffreestanding -mno-red-zone -mcmodel=large -nostdlib -m64 \
		 -I$(INCLUDE_PATH) 
LDFLAGS = -L$(LIBRARY_PATH) -nostdlib -nodefaultlibs -m elf_x86_64 -T link.ld -lcorelib --traditional-format

KERNELNAME = kernel
KERNEL = $(addprefix $(BIN)/, $(KERNELNAME))
OBJS = *.o
AS = nasm
ASFLAGS = -f elf64

all: $(KERNEL)

OBJECTS = kernel.o monitor.o io.o gdt.o idt.o isr.o gdt_flush.o memman.o msr.o interrupt.o ioapic.o apic.o diskio.o readdisk.o fat32.o acpi.o


$(KERNEL): $(OBJECTS) link.ld
	$(LD) $^ $(LDFLAGS) -o $@
	strip $@
	-rm $(OBJS)

kernel.o: main.c
	$(CC) -c $(CFLAGS) $< -o $@

acpi.o: acpi.c
	$(CC) -c $(CFLAGS) $< -o $@

ioapic.o: ioapic.c
	$(CC) -c $(CFLAGS) $< -o $@

apic.o: apic.c
	$(CC) -c $(CFLAGS) $< -o $@

msr.o: msr.c
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

fat32.o: fat32.c
	$(CC) -c $(CFLAGS) $< -o $@

diskio.o: diskio.c
	$(CC) -c $(CFLAGS) $< -o $@
    
readdisk.o: diskio.asm
	$(AS) $(ASFLAGS) -o $@ $< 

memman.o: memman.c
	$(CC) -c $(CFLAGS) $< -o $@

gdt_flush.o: gdt_flush.asm
	$(AS) $(ASFLAGS) -o $@ $< 

interrupt.o: interrupt.asm
	$(AS) $(ASFLAGS) -o $@ $< 

clean:
	-rm $(KERNEL) $(OBJS) defaultlibs
