BIN = ../bin
VPATH = .
INCLUDE_PATH = -I../include -I.
LIBRARY_PATH = -L../lib
CC = gcc
LD = ld
CFLAGS = -ffreestanding -mcmodel=large -nostdlib -m64 \
		 $(INCLUDE_PATH) $(LIBRARY_PATH)
LDFLAGS = -nostdlib -nodefaultlibs -m elf_x86_64 -T link.ld

KERNELNAME = kernel
KERNEL = $(addprefix $(BIN)/, $(KERNELNAME))
OBJS = *.o

all: $(KERNEL)

$(KERNEL): kernel.o monitor.o io.o link.ld
	$(LD) $(LDFLAGS) $^ -o $@

kernel.o: main.c
	$(CC) -c $(CFLAGS) $< -o $@

monitor.o: monitor.c
	$(CC) -c $(CFLAGS) $< -o $@

io.o: io.c
	$(CC) -c $(CFLAGS) $< -o $@


clean:
	-rm $(KERNEL) $(OBJS)
