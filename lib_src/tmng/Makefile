# @file $RCSfile: Makefile,v $
#  general description of this module
#  $Id: Makefile 9911 2010-06-25 10:39:35Z tomaswang $
#  @author $Author: tomaswang $
#  @date $Date: 2010-06-25 18:39:35 +0800 (Fri, 2010-06-25) $
#  @version 1.0
#  @note Editor: Vim 6.1, Gcc 4.0.2, tab=4
#  @note Platform: Linux

include ../../tsf4g.mk

MNG_CFILE=$(wildcard *.c)
MNG_OBJ=$(MNG_CFILE:.c=.o)


COMMLIB = $(TSF4G_LIB)/libcomm.a
PALLIB= $(TSF4G_LIB)/libpal.a
DRLIB= $(TSF4G_LIB)/libtdr_xml.a -lexpat -lscew
TBUSLIB=-ltbus
LIBS =  $(DRLIB)  $(PALLIB) $(COMMLIB)

MNG_LIB=$(TSF4G_LIB)/libtsm.a
MNG_SO=$(TSF4G_LIB)/libtsm.so

CFLAGS:=$(CFLAGS) -I$(TSF4G_LIBSRC)

.PHONY: all clean

all: $(MNG_LIB) $(MNG_SO)
	cp $(MNG_OBJ) $(TSF4G_LIB_PUB_OBJ_DIR)
	#cp $(MNG_OBJ) $(TSF4G_SO_OBJ_DIR)
	
.c.o:
	$(CC) -c $(CFLAGS) $(SHARED) $(DEBUG) *.c

$(MNG_LIB): $(MNG_OBJ)
	ar r $(MNG_LIB) $(MNG_OBJ)
	

$(MNG_SO): $(MNG_OBJ)
	$(CC) -o $(MNG_SO) $(SHARED) $(CFLAGS) $(DEBUG) $^ $(LDPATH) $(LIBS)


clean:
	$(RM) *.o 
	$(RM) $(MNG_LIB) $(MNG_SO)
