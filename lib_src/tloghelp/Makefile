#  @file $RCSfile: Makefile,v $
#  general description of this module
#  $Id: Makefile,v 1.3 2009-08-18 11:30:05 jacky Exp $
#  @author $Author: jacky $
#  @date $Date: 2009-08-18 11:30:05 $
#  @version $Revision: 1.3 $

include ../../tsf4g.mk



TLOGHELP_CFILE=$(wildcard *.c)
TLOGHELP_OBJ=$(TLOGHELP_CFILE:.c=.o)
TLOGHELP_LIB=$(TSF4G_LIB)/libtloghelp.a
TLOGHELP_SO=$(TSF4G_LIB)/libtloghelp.so

CFLAGS += -DVERSION="\"1.2.1\""

LIBS  +=  $(TSF4G_LIB)/libtmng.a $(TSF4G_LIB)/libtlog.a $(TSF4G_LIB)/libtdr_xml.a $(TSF4G_LIB)/libpal.a $(TSF4G_LIB)/libcomm.a  $(TSF4G_XML_LIBS)


ifeq ($(debug), 1)
	CFLAGS += -D__SD_DEBUG__ -D__LOG4C_DEBUG__
endif

.PHONY: all clean

all: $(TLOGHELP_LIB) $(TLOGHELP_SO)
	cp $(TLOGHELP_OBJ)  $(TSF4G_LIB_PUB_OBJ_DIR)
	#cp $(TLOGHELP_OBJ)  $(TSF4G_SO_OBJ_DIR)

-include $(TLOGHELP_CFILE:.c=.d)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $(DEBUG) $< -o $@


$(TLOGHELP_LIB): $(TLOGHELP_OBJ) 
	ar r $@ $^
	

$(TLOGHELP_SO): $(TLOGHELP_OBJ) 
	$(CC) -o $@ $(SHARED) $(CFLAGS) $(DEBUG) $^ $(LDPATH) $(LIBS)  

clean:
	$(RM) *.o *.d
	$(RM)  $(TLOGHELP_LIB) $(TLOGHELP_SO)

