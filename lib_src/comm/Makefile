# @file $RCSfile: Makefile,v $
#  general description of this module
#  $Id: Makefile,v 1.3 2009-08-18 11:29:56 jacky Exp $
#  @author $Author: jacky $
#  @date $Date: 2009-08-18 11:29:56 $
#  @version 1.0
#  @note Editor: Vim 6.1, Gcc 4.0.2, tab=4
#  @note Platform: Linux

include ../../tsf4g.mk

COMM_CFILE=$(wildcard *.c)
COMM_OBJ=$(COMM_CFILE:.c=.o)
COMM_LIB=$(TSF4G_LIB)/libcomm.a
COMM_SO=$(TSF4G_LIB)/libcomm.so
COMM_IMPORTS_LIB = $(TSF4G_LIB)/libpal.a $(TSF4G_XML_LIBS)  -lpthread

.PHONY: all clean

all: $(COMM_LIB) $(COMM_SO)
	cp $(COMM_OBJ)  $(TSF4G_LIB_PUB_OBJ_DIR)
	#cp $(COMM_OBJ)  $(TSF4G_LIB_PUB_OBJ_DIR)
	
-include $(COMM_CFILE:.c=.d)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@
	

%.o: %.c %.d
	$(CC) -c  $(CFLAGS) $<

$(COMM_LIB):	$(COMM_OBJ)
	ar r $(COMM_LIB) $^
	

$(COMM_SO): $(COMM_OBJ)
	$(CC) -o $(COMM_SO) $(SHARED) $(CFLAGS)  $^ $(COMM_IMPORTS_LIB)

clean:
	$(RM) *.o  *.d
	$(RM) $(COMM_LIB)
	$(RM) $(COMM_SO)

