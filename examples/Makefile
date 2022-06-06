#  @file $RCSfile: Makefile,v $
#  general description of this module
#  $Id: Makefile,v 1.3 2009-03-31 03:06:59 kent Exp $
#  @author $Author: kent $
#  @date $Date: 2009-03-31 03:06:59 $
#  @version 1.0
#  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
#  @note Platform: Linux

include  ../tsf4g.mk

CLEANTARGET = clean


MODS = comm tdr tbus tlog tapp
MODSCLEAN =$(patsubst %, %.$(CLEANTARGET), $(MODS))
CUS_PATH='pwd'

all: $(MODS)

.PHONY:  all  clean $(MODS) $(MODSCLEAN)

$(MODS):
	@echo "Begin to make tsf4g module: '$@' ......"
	cd $@ && $(MAKE);
	@echo "Finish to  make tsf4g module: '$@'"

$(MODSCLEAN):
	@echo "Begin to clean tsf4g module: '$(patsubst %.$(CLEANTARGET),%, $@)' ......"
	cd $(patsubst %.$(CLEANTARGET),%, $@) && $(MAKE) $(CLEANTARGET);
	@echo "Finish to clean tsf4g module: '$(patsubst %.$(CLEANTARGET),%, $@)'"

$(CLEANTARGET): $(MODSCLEAN)
test:
	cd tlog && make test
