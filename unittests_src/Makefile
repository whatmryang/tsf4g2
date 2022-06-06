#  @file $RCSfile: Makefile,v $
#  general description of this module
#  $Id: Makefile,v 1.3 2008-11-17 01:26:52 jackyai Exp $
#  @author $Author: jackyai $
#  @date $Date: 2008-11-17 01:26:52 $
#  @version 1.0
#  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
#  @note Platform: Linux

include  ../tsf4g.mk

CLEANTARGET = clean

MODS = pal tdr tmng
#ifeq ($(withsec), yes)
#MODS += sec
#endif

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

unittest:
	cd pal && $(MAKE) test;
	cd tdr && $(MAKE) test;
	cd tmng && $(MAKE) test;
	cd sec && $(MAKE) test;

test:
	echo "Add test targets here."

$(CLEANTARGET): $(MODSCLEAN)

