#  @file $RCSfile: Makefile,v $
#    Makefile for tsf4g-tbus relay module
#  $Id: Makefile,v 1.3 2009-03-30 07:37:42 jacky Exp $
#  @author $Author: jacky $
#  @date $Date: 2009-03-30 07:37:42 $
#  @version $Revision: 1.3 $
#  @note Editor: Vim 6.3, Gcc 4.0.2, tab=4
#  @note Platform: Linux

include ../../tsf4g.mk

TARGETS = \
	SHELL  testsend

all: $(TARGETS)

SHELL:
	@./generate.sh $(TSF4G_TOOLS)
	@$(MAKE) -f Makefile.real
	
testsend:
	cd test.send && $(MAKE);
	
#------------------------------------------------------
clean: shellclean testsendclean

shellclean:
	@$(MAKE) -f Makefile.real clean

testsendclean:
	cd test.send && $(MAKE) clean;

