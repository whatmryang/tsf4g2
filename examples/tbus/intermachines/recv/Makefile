#  @file $RCSfile: Makefile,v $
#    Makefile for tsf4g-tbus test tool
#  $Id: Makefile,v 1.2 2009/09/16 07:20:47 jacky Exp $
#  @author $Author: jacky $
#  @date $Date: 2009/09/16 07:20:47 $
#  @version $Revision: 1.2 $
#  @note Editor: Vim 6.3, Gcc 4.0.2, tab=4
#  @note Platform: Linux

include ../../../../tsf4g.mk


TBUS_TEST_CFILE=$(wildcard *.c)
TBUS_TEST_OBJ=$(TBUS_TEST_CFILE:.c=.o)
TEST_TOOL=test

CINC += -I../

LIBS = $(LDPATH) -ltsf4g  -Wl,-Bstatic -lscew -lexpat -Wl,-Bdynamic  -lpthread 

.PHONY: all clean

all: $(TEST_TOOL)

-include $(TBUS_TEST_CFILE:.c=.d)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $(DEBUG) $<

$(TEST_TOOL): $(TBUS_TEST_OBJ)
	$(CC) -o $@ $^ $(LDPATH) $(LIBS)

clean:
	$(RM) core *.o  *.d
	$(RM) $(TEST_TOOL)

