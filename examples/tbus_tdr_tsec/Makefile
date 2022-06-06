#  @file $RCSfile: Makefile,v $
#  general description of this module
#  $Id: Makefile,v 1.3 2009-09-04 08:01:34 sean Exp $
#  @author $Author: sean $
#  @date $Date: 2009-09-04 08:01:34 $
#  @version $Revision: 1.3 $
#  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
#  @note Platform: Linux
#
CLEANTARGET = clean
SUBMODS = tbustestserver
SUBMODSCLEAN =$(patsubst %, %.$(CLEANTARGET), $(SUBMODS))

.PHONY: all test $(CLEANTARGET) $(SUBMODS)

all: $(SUBMODS)
$(CLEANTARGET): $(SUBMODSCLEAN)

$(SUBMODS):
	@echo "Begin to make sub module: '$@' ......"
	cd $@ && $(MAKE);
	@echo "Finish to  make sub module: '$@'"

$(SUBMODSCLEAN):
	@echo "Begin to clean sub module: '$(patsubst %.$(CLEANTARGET),%, $@)' ......"
	cd $(patsubst %.$(CLEANTARGET),%, $@)  && make $(CLEANTARGET);
	@echo "Finish to  clean sub module: '$(patsubst %.$(CLEANTARGET),%, $@)'"
test:all
	./test.sh

