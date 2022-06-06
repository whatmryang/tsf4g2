#  @file $RCSfile: Makefile,v $
#    Makefile for tsf4g-tbus test tool
#  $Id: Makefile,v 1.3 2009/09/16 07:20:48 jacky Exp $
#  @author $Author: jacky $
#  @date $Date: 2009/09/16 07:20:48 $
#  @version $Revision: 1.3 $
#  @note Editor: Vim 6.3, Gcc 4.0.2, tab=4
#  @note Platform: Linux


include ../../../tsf4g.mk

TBUS_TEST_CFILE=$(wildcard *.c)
TBUS_TEST_OBJ=$(TBUS_TEST_CFILE:.c=.o)
TEST_TOOL=send recv trsf

CFLAGS += -I../ -I$(TSF4G_INC)

LDLIBS += $(LDPATH) $(LOG_LIB) $(TSF4G_LIB)/libtsf4g.a -Wl,-Bstatic -lscew -lexpat -Wl,-Bdynamic  -lpthread -lmysqlclient
.PHONY: all clean

all: $(TEST_TOOL)
-include $(subst .c,.d,$(TBUS_TEST_CFILE))

%.d: %.c
	$(CC) -MM $(CFLAGS) $< > $@

clean:
	$(RM) core *.o  *.d
	$(RM) $(TEST_TOOL)

