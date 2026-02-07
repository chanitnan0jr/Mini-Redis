# Root Makefile

.PHONY: all clean examples miniredis

all: examples miniredis

examples:
	$(MAKE) -C examples/tcp
	$(MAKE) -C examples/udp
	$(MAKE) -C examples/utils

miniredis:
	$(MAKE) -C miniredis

clean:
	$(MAKE) -C examples/tcp clean
	$(MAKE) -C examples/udp clean
	$(MAKE) -C examples/utils clean
	$(MAKE) -C miniredis clean