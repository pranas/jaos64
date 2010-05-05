BIN = ../bin
INCLUDE_PATH = ../include
LIBRARY_PATH = ../lib
CC = gcc
LD = ld
CFLAGS = -Wall -ffreestanding -mno-red-zone -mcmodel=large -nostdlib -m64 \
		 -I$(INCLUDE_PATH) 
LDFLAGS = -L$(LIBRARY_PATH) -nostdlib -nodefaultlibs -m elf_x86_64 -T link.ld -lcorelib --traditional-format

KERNELNAME = kernel
KERNEL = $(addprefix $(BIN)/, $(KERNELNAME))
OBJS = *.o
AS = nasm
ASFLAGS = -f elf64

all: $(KERNEL)

OBJECTS = main.o readdisk.o interrupt.o gdt_flush.o monitor.o io.o gdt.o idt.o isr.o \
		  memman.o ioapic.o apic.o acpi.o msr.o diskio.o fat32.o keyboard.o usermode.o \
		  syscall.o


$(KERNEL): $(OBJECTS) link.ld
	$(LD) $^ $(LDFLAGS) -o $@
	strip $@
	-rm $(OBJECTS)

%.o: %.asm
	$(AS) $(ASFLAGS) -o $@ $<

clean:
	-rm $(KERNEL) $(OBJECTS) defaultlibs
