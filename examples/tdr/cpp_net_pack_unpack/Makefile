#  @file $RCSfile: Makefile,v $
#    Makefile wrapper for tsf4g-tbus module
#  $Id: Makefile,v 1.6 2008-11-12 09:53:08 kent Exp $
#  @author $Author: kent $
#  @date $Date: 2008-11-12 09:53:08 $
#  @version $Revision: 1.6 $
#  @note Editor: Vim 6.3, Gcc 4.0.2, tab=4
#  @note Platform: Linux


TARGETS = \
	SHELL \

all: $(TARGETS)
SHELL:
	../../../tools/tdr -P net_protocol.xml net_msg_type.xml -O .
	@$(MAKE) -f Makefile.real

#------------------------------------------------------
clean:
	@$(MAKE) -f Makefile.real clean

