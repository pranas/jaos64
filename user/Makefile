BIN = ../bin
INCLUDE_PATH = ../include
LIBRARY_PATH = ../lib
CC = gcc
CFLAGS = -ffreestanding -mno-red-zone -mcmodel=large -nostdlib -m64 \
		 -I$(INCLUDE_PATH) 
LD = ld
LDFLAGS = -L$(LIBRARY_PATH) -nostdlib -nodefaultlibs -m elf_x86_64 --traditional-format -lcorelib
SRCS = loop shell fork ping hello
OUTPUT = $(addprefix $(BIN)/, $(SRCS:%=%))

all: $(OUTPUT)

$(BIN)/hello: hello.o syscall.o
	$(LD) $^ $(LDFLAGS) -o $@

$(BIN)/ping: ping.o syscall.o
	$(LD) $^ $(LDFLAGS) -o $@

$(BIN)/fork: fork.o syscall.o
	$(LD) $^ $(LDFLAGS) -o $@

$(BIN)/shell: shell.o syscall.o
	$(LD) $^ $(LDFLAGS) -o $@

$(BIN)/%: %.o
	$(LD) $^ $(LDFLAGS) -o $@

clean:
	-@rm -f *.o
