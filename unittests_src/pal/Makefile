# @file $RCSfile: Makefile,v $
#  general description of this module
#  $Id: Makefile,v 1.1.1.1 2008-05-28 07:35:00 kent Exp $
#  @author $Author: kent $
#  @date $Date: 2008-05-28 07:35:00 $
#  @version 1.0
#  @note Editor: Vim 6.1, Gcc 4.0.2, tab=4
#  @note Platform: Linux

include ../../tsf4g.mk

APP_CFILE=$(wildcard *.c)
APP_OBJ=$(APP_CFILE:.c=.o)
APP_NAME=$(TSF4G_UNITTESTS)/pal/ut_pal

.PHONY: all clean

all: $(APP_NAME)

-include $(APP_CFILE:.c=.d)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $(DEBUG)  $<

$(APP_NAME): $(APP_OBJ)
	$(CC) -o $(APP_NAME) $(CFLAGS) $(DEBUG) $^  $(LDPATH)  -lpal  -lcunit -lpthread $(LTSTATIC)

test:$(APP_NAME)
	./$(APP_NAME)
	
clean:
	$(RM) *.o  *.d
	$(RM) $(APP_NAME)

