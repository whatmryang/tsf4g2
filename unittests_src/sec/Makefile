#@file $RCSfile: Makefile,v $
#  general description of this module
#  $Id: Makefile,v 1.1.1.1 2008-05-28 07:35:00 kent Exp $
#@author $Author: kent $
#@date $Date: 2008-05-28 07:35:00 $
#@version $Revision: 1.1.1.1 $
#@note Editor: Vim 6.3, Gcc 4.0.2, tab=4
#@note Platform: Linux


include ../../tsf4g.mk

TSEC_CFILE=$(wildcard *.c)
TSEC_OBJ=$(TSEC_CFILE:.c=.o)

#CINC += -I../ -I../../../deps/CUnit/CUnit/Headers/
LDPATH += -L$(TSF4G_LIB)
LIBS += -lcunit -ltsec -ltlog -ltdr_xml -lpal

.PHONY: all clean
UT_BIN = $(TSF4G_UNITTESTS)/sec/ut_tsec
TARGETS = \
	$(UT_BIN) \

all: $(TARGETS)

-include $(TSEC_CFILE:.c=.d)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $(DEBUG) $<

$(UT_BIN): $(TSEC_OBJ)
	$(CC) -o $@ $^ $(LDPATH) $(LIBS)

test:ut_tsec
	./ut_tsec

	
clean:
	$(RM) *.o core *.d
	$(RM) $(TARGETS)

