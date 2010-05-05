INCLUDE_PATH = ../include

CC = gcc
CFLAGS = -ffreestanding -mcmodel=large -nodefaultlibs -nostartfiles -nostdlib -m64 \
        -I$(INCLUDE_PATH) 
AR = ar
ARFLAGS = rcsv

OBJECTS = stdlib.o string.o

all: libcorelib.a

libcorelib.a: $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

clean:
	-rm libcorelib.a $(OBJECTS)
