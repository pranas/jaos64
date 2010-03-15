all:
	$(MAKE) -C boot/
	$(MAKE) -C kernel/
	$(MAKE) -C bin/
	@echo -e '\033[0;32m---- BUILD SUCCESSFUL ----\033[m'

clean:
	$(MAKE) clean -C boot/
	$(MAKE) clean -C kernel/
	$(MAKE) clean -C bin/

run:
	bochs -q -rc dbgrc -f bochsrc
