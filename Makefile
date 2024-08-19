# Makefile - gccDppConsole

.PHONY: all
all: \
 	gccDppConsole


.PHONY: gccDppConsole
gccDppConsole:
	$(MAKE) -f gccDppConsole.mak

.PHONY: clean
clean:
	$(MAKE) -f gccDppConsole.mak clean

.PHONY: depends
depends:
	$(MAKE) -f gccDppConsole.mak depends

