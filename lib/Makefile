INCLUDE_PATH = ../include

CC = gcc
CFLAGS = -ffreestanding -mcmodel=large -nodefaultlibs -nostartfiles -nostdlib -m64 \
		 -I$(INCLUDE_PATH) 
AR = ar
ARFLAGS = rcsv

all: libcorelib.a

libcorelib.a: corelib.o
	$(AR) $(ARFLAGS) $@ $^

corelib.o: stdlib.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	-rm libcorelib.a corelib.o
