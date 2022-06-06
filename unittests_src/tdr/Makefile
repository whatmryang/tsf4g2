#  @file $RCSfile: Makefile,v $
#    Makefile wrapper for tsf4g-tbus module
#  $Id: Makefile,v 1.3 2009-08-18 11:23:53 jacky Exp $
#  @author $Author: jacky $
#  @date $Date: 2009-08-18 11:23:53 $
#  @version $Revision: 1.3 $
#  @note Editor: Vim 6.3, Gcc 4.0.2, tab=4
#  @note Platform: Linux


include ../../tsf4g.mk

TARGETS = \
	SHELL \

all: $(TARGETS)

SHELL:
	@./generate.sh $(TSF4G_TOOLS)
	@$(MAKE) -f Makefile.real


#------------------------------------------------------
clean:
	@$(MAKE) -f Makefile.real clean


