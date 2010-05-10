BIN = ../bin
INCLUDE_PATH = ../include
LIBRARY_PATH = ../lib
CC = gcc
LD = ld
CFLAGS = -Wall -Wextra -ffreestanding -mno-red-zone -mcmodel=large -nostdlib -m64 \
		 -I$(INCLUDE_PATH) 
LDFLAGS = -L$(LIBRARY_PATH) -nostdlib -nodefaultlibs -m elf_x86_64 -T link.ld -lcorelib --traditional-format

KERNELNAME = kernel
KERNEL = $(addprefix $(BIN)/, $(KERNELNAME))
AS = nasm
ASFLAGS = -f elf64

all: $(KERNEL)

OBJECTS = main.o monitor.o io.o gdt.o idt.o isr.o \
			ioapic.o apic.o acpi.o msr.o diskio.o fat32.o keyboard.o memman.o  \
			syscall.o scheduler.o elf.o kheap.o ordered_array.o fork.o  b_locking.o \
			read_sectors.o read_rip.o spin_lock.o switch_to_user_mode.o gdt_flush.o \
			interrupt.o 

$(KERNEL): $(OBJECTS) link.ld
	$(LD) $^ $(LDFLAGS) -o $@
	strip $@
	@rm $(OBJECTS)

%.o: %.asm
	$(AS) $(ASFLAGS) -o $@ $<

clean:
	@rm $(OBJECTS) defaultlibs
