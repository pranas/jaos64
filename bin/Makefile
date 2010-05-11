# Resources used:
#	http://kbarr.net/bochs
#

IMAGE = hdd.img
CONTENT = stage2 loop kernel shell fork
LODEV = $(shell losetup -f)

all: $(IMAGE) format mbr

$(IMAGE):
ifneq ($(shell id -u), 0)
	@echo "This Makefile must be run as root"
	@exit 1
else
	@bximage -hd -mode=flat -size=50 -q $(IMAGE) > /dev/zero
#fdisk requires root
	-@fdisk $(IMAGE) < fdisk.script &> /dev/zero
endif

format:
ifneq ($(shell id -u), 0)
	@echo "This Makefile must be run as root"
	@exit 1
else
	@losetup -o 512 $(LODEV) $(IMAGE)
	@mkdosfs -F 32 -s 1 -n Project $(LODEV) > /dev/zero
#-s sectors-per-cluster
	@mkdir tmp
	@mount -t vfat $(LODEV) tmp/
	-cp $(CONTENT) tmp/
	@umount $(LODEV)
	@rm -r -f tmp/
	@losetup -d $(LODEV)
	@chown :users $(IMAGE)
	@chmod 775 $(IMAGE)
endif

mbr: stage1
	dd if=stage1 of=$(IMAGE) bs=446 count=1 conv=notrunc

clean:
	@rm $(IMAGE)
