#  @file $RCSfile: Makefile,v $
#    Makefile wrapper for tsf4g-tbus module
#  $Id: Makefile,v 1.4 2009-08-18 09:39:36 jacky Exp $
#  @author $Author: jacky $
#  @date $Date: 2009-08-18 09:39:36 $
#  @version $Revision: 1.4 $
#  @note Editor: Vim 6.3, Gcc 4.0.2, tab=4
#  @note Platform: Linux


include ../../tsf4g.mk

TARGETS = \
	SHELL \

all: $(TARGETS)

SHELL:
	@$(MAKE) -f Makefile.st
	@$(MAKE) -f Makefile.so

#------------------------------------------------------
clean:
	@$(MAKE) -f Makefile.st   clean
	@$(MAKE) -f Makefile.so     clean

