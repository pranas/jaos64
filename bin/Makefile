OBJECTS = stage1 stage2 kernel
SIZE = $(shell for i in $(OBJECTS); do sum=$$(( $$sum + `du -b "$$i" | awk '{print $$1 }'`)); done; \
   	echo $$(( ((1440*1024) - $$sum) / 512 )) )

all: floppy.img

floppy.img: $(OBJECTS) pad
	cat $^ > $@

pad:
	dd if=/dev/zero of=$@ bs=512 count=$(SIZE)

# minus sign means to ignore errors
clean:
	-rm floppy.img pad
