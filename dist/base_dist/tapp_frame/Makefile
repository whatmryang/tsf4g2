#  @file $RCSfile: Makefile,v $
#  general description of this module
#  $Id: Makefile,v 1.1 2009-09-15 10:33:49 flyma Exp $
#  @author $Author: flyma $
#  @date $Date: 2009-09-15 10:33:49 $
#  @version $Revision: 1.1 $
#  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
#  @note Platform: Linux
#

TSF4G_HOME=${TSF4G_INSTALL_HOME}
TSF4G_INC  = $(TSF4G_HOME)/include
TSF4G_SERVICES = $(TSF4G_HOME)/services
TSF4G_LIB  = $(TSF4G_HOME)/lib
TSF4G_TOOLS = $(TSF4G_HOME)/tools




TARGETS = \
	SHELL \

all: $(TARGETS)

SHELL:
	@./conv.sh $(TSF4G_TOOLS)
	@$(MAKE) -f Makefile.real

#------------------------------------------------------
clean:
	@$(MAKE) -f Makefile.real clean
