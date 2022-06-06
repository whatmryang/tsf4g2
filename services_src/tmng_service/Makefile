# @file $RCSfile: Makefile,v $
#  general description of this module
#  $Id: Makefile,v 1.4 2009-08-20 07:39:57 jacky Exp $
#  @author $Author: jacky $
#  @date $Date: 2009-08-20 07:39:57 $
#  @version 1.0
#  @note Editor: Vim 6.1, Gcc 4.0.2, tab=4
#  @note Platform: Linux

include ../../tsf4g.mk

MNG_CFILE=$(wildcard *.c)
MNG_OBJ=$(MNG_CFILE:.c=.o)


COMMLIB = $(TSF4G_LIB)/libcomm.a
PALLIB= $(TSF4G_LIB)/libpal.a
DRLIB= $(TSF4G_LIB)/libtdr_xml.a -lexpat -lscew
TBUSLIB=$(TSF4G_LIB)/libtbus.a
TMNGLIB=$(TSF4G_LIB)/libtmng.a
LIBS = $(TMNGLIB) $(TBUSLIB) $(TSF4G_LIB)/libtagentapi.a $(TSF4G_LIB)/libtlog.a \
$(DRLIB)  $(PALLIB) $(COMMLIB) -lpthread

MNGSVC_LIB=$(TSF4G_LIB)/libtmngsvc.a
MNGSVC_SO=$(TSF4G_LIB)/libtmngsvc.so

.PHONY: all clean

all: $(MNGSVC_LIB) $(MNGSVC_SO)

.c.o:
	$(CC) -c $(CFLAGS) $(SHARED) $(DEBUG) *.c

$(MNGSVC_LIB): $(MNG_OBJ)
	ar r $(MNGSVC_LIB) $(MNG_OBJ)
	

$(MNGSVC_SO): $(MNG_OBJ)
	$(CC) -o $(MNGSVC_SO) $(SHARED) $(CFLAGS) $(DEBUG) $^ $(LDPATH) $(LIBS)


clean:
	$(RM) *.o 
	$(RM) $(MNGSVC_LIB) $(MNGSVC_SO)
