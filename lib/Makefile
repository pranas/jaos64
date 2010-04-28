INCLUDE_PATH = ../include

CC = gcc
CFLAGS = -ffreestanding -mcmodel=large -nodefaultlibs -nostartfiles -nostdlib -m64 \
        -I$(INCLUDE_PATH) 
AR = ar
ARFLAGS = rcsv

all: libcorelib.a

libcorelib.a: stdlib.o string.o
	$(AR) $(ARFLAGS) $@ $^

stdlib.o: stdlib.c
	$(CC) $(CFLAGS) -o $@ -c $^

string.o: string.c
	$(CC) $(CFLAGS) -o $@ -c $^

clean:
	-rm libcorelib.a corelib.o string.o stdlib.o
