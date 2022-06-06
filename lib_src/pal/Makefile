# @file $RCSfile: Makefile,v $
#  general description of this module
#  $Id: Makefile,v 1.3 2009-08-18 11:29:59 jacky Exp $
#  @author $Author: jacky $
#  @date $Date: 2009-08-18 11:29:59 $
#  @version 1.0
#  @note Editor: Vim 6.1, Gcc 4.0.2, tab=4
#  @note Platform: Linux

include ../../tsf4g.mk
include Makefile.pub


PAL_CFILE=$(wildcard *.c)
PAL_OBJ=$(PAL_CFILE:.c=.o)
PAL_LIB=$(TSF4G_LIB)/libpal.a
PAL_SO=libpal.so
PAL_SO_RALATIVE=$(PAL_SO).${PAL_MAJOR}.${PAL_MINOR}.$(PAL_PATCH)
PAL_SO_REAL=$(TSF4G_LIB)/$(PAL_SO_RALATIVE)
PAL_SO_NAME=$(PAL_SO).${PAL_MAJOR}

.PHONY: all clean

all: $(PAL_LIB) $(PAL_SO_REAL)
	cp $(PAL_OBJ)  $(TSF4G_LIB_PUB_OBJ_DIR)
	#cp $(PAL_OBJ)  $(TSF4G_LIB_OBJ_DIR)
	@$(LN) $(TSF4G_LIB)  $(PAL_SO_RALATIVE) $(PAL_SO_NAME) 
	@$(LN) $(TSF4G_LIB) $(PAL_SO_NAME) $(PAL_SO)

-include $(PAL_CFILE:.c=.d)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS)  $<

$(PAL_LIB):	$(PAL_OBJ)
	ar r $(PAL_LIB) $^
	

$(PAL_SO_REAL): $(PAL_OBJ)
	$(CC) -o $(PAL_SO_REAL) $(SHARED) $(CFLAGS) -Wl,-soname,$(PAL_SO_NAME)  $^ 

clean:
	$(RM) *.o  *.d
	$(RM) $(PAL_LIB)
	$(RM) $(PAL_SO)

