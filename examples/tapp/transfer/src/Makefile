
include ../transfer.mk

TARGETS = SHELL

all: $(TARGETS)

SHELL:
	@./conv.sh $(TSF4G_TOOLS)
	@$(MAKE) -f Makefile.real

clean:
	@$(MAKE) -f Makefile.real clean
