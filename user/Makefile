BIN = ../bin
INCLUDE_PATH = ../include
LIBRARY_PATH = ../lib
CC = gcc
CFLAGS = -ffreestanding -mno-red-zone -mcmodel=large -nostdlib -m64 \
		 -I$(INCLUDE_PATH) 
LD = ld
LDFLAGS = -L$(LIBRARY_PATH) -nostdlib -nodefaultlibs -m elf_x86_64 --traditional-format
SRCS = loop
OUTPUT = $(addprefix $(BIN)/, $(SRCS:%=%))

all: $(OUTPUT)

$(BIN)/%: %.o
	$(LD) $^ $(LDFLAGS) -o $@

clean:
	-@rm $(OUTPUT)
	-@rm -f *.o