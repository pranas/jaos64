all:
	$(MAKE) -C lib/
	$(MAKE) -C boot/
	$(MAKE) -C kernel/
	$(MAKE) -C user/
	$(MAKE) -C bin/
	$(MAKE) clean -C lib/
	$(MAKE) clean -C user/
	@echo -e '\033[0;32m---- BUILD SUCCESSFUL ----\033[m'

clean:
	$(MAKE) clean -C lib/
	$(MAKE) clean -C boot/
	$(MAKE) clean -C kernel/
	$(MAKE) clean -C user/
	$(MAKE) clean -C bin/

run:
	bochs -q -rc dbgrc -f bochsrc
