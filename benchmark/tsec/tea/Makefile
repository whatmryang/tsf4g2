#  general description of this module
#  $Id: Makefile,v 1.2 2009-08-17 06:58:35 hardway Exp $
#  @author $Author: hardway $
#  @date $Date: 2009-08-17 06:58:35 $
#  @version 1.0
#  @note Editor: Vim 6.1, Gcc 4.0.2, tab=4
#  @note Platform: Linux

include ../../../tsf4g.mk

TDR_CFILE=$(wildcard *.c)
TDR_OBJ=$(TDR_CFILE:.c=.o)


TDR_BIN= testteaproformance


TDR_IMPORT_LIBS = $(LDPATH)  ${TSF4G_LIB}/libtsec.a    
.PHONY: all clean

all: $(TDR_BIN)



%.o: %.c 
	$(CC) -o $@ $(CFLAGS)   -c $<

	
$(TDR_BIN): $(TDR_OBJ)
	$(CC) -o $@  $(CFLAGS)    $^  $(TDR_IMPORT_LIBS) -static
	


clean:
	$(RM) *.o  *.d
	$(RM) $(TDR_BIN)
