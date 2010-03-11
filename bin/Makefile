SIZE = $(shell for i in $(OBJECTS); \
	   do sum=$$(( $$sum + `du -b "$$i" | awk '{print $$1 }'`)); \
	   done; \
	   echo $$(( ((1440*1024) - $$sum) / 512 )) \
	   )
OBJECTS = stage1.bin stage2.bin kernel

all: floppy.img

floppy.img: $(OBJECTS) pad
	cat $^ > $@

pad:
	dd if=/dev/zero of=$@ bs=512 count=$(SIZE)

# minus sign means to ignore errors
clean:
	-rm pad
