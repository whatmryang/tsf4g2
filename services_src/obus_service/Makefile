#  @file $RCSfile: Makefile,v $
#    Makefile for tsf4g-tbus relay module
#  $Id: Makefile,v 1.1.1.1 2008-05-28 07:35:00 kent Exp $
#  @author $Author: kent $
#  @date $Date: 2008-05-28 07:35:00 $
#  @version $Revision: 1.1.1.1 $
#  @note Editor: Vim 6.3, Gcc 4.0.2, tab=4
#  @note Platform: Linux

include ../../tsf4g.mk

CFLAGS += -I$(TSF4G_INC)/obus 

OBUS_RELAY_CFILE=$(wildcard *.c)
OBUS_RELAY_OBJ=$(OBUS_RELAY_CFILE:.c=.o)
OBUS_RELAY_BIN=obus_relay_client obus_relay_server


.PHONY: all clean

all:	$(OBUS_RELAY_BIN) 
	
-include $(OBUS_RELAY_CFILE:.c=.d)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $(DEBUG) $< 

obus_relay_server:obus_relay.o obus_relay_server.o 
	$(CC) $(STATIC) -o $@ $^ $(TSF4G_LIB)/libobus.a
	cp obus_relay_server $(TSF4G_SERVICES)/obus/
obus_relay_client:obus_relay.o obus_relay_client.o
	$(CC) $(STATIC) -o $@ $^ $(TSF4G_LIB)/libobus.a
	cp obus_relay_client $(TSF4G_SERVICES)/obus/
	
clean:
	$(RM) core *.o  *.d $(OBUS_RELAY_BIN)
