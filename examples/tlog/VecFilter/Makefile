#  @file $RCSfile: Makefile,v $
#  general description of this module
#  $Id: Makefile,v 1.1 2009-03-30 03:40:40 kent Exp $
#  @author $Author: kent $
#  @date $Date: 2009-03-30 03:40:40 $
#  @version $Revision: 1.1 $
#  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
#  @note Platform: Linux
#

include ../../../tsf4g.mk

APP_NAME=VecFilter

APP_CFILE=$(wildcard *.c)
APP_OBJ=$(APP_CFILE:.c=.o)

TLOG_IMPORT_LIBS= $(LDPATH) $(TSF4G_LIB)/libtsf4g.a -Wl,-Bstatic -lscew -lexpat -Wl,-Bdynamic  -lpthread

.PHONY: all clean

all: $(APP_NAME)

-include $(APP_CFILE:.c=.d)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $<

$(APP_NAME): $(APP_OBJ)
	$(CC)  -o $(APP_NAME) $(CFLAGS)  $^ $(TLOG_IMPORT_LIBS)  

clean:
	$(RM) *.o  *.d
	$(RM) $(APP_NAME) *.log 
test:
	./test.sh
