#  general description of this module
#  $Id: Makefile,v 1.6 2009-09-09 10:34:03 flyma Exp $
#  @author $Author: flyma $
#  @date $Date: 2009-09-09 10:34:03 $
#  @version 1.0
#  @note Editor: Vim 6.1, Gcc 4.0.2, tab=4
#  @note Platform: Linux

include ../../../tsf4g.mk

TDR_CPPFILE=$(wildcard *.cpp)
CXX=g++

TDR_TOOLS_CFLAGS = $(CFLAGS) $(TSF4G_DBMS_INCLUDE) -I $(TSF4G_LIBSRC)/tdr

TDR_METACHECK=$(TSF4G_TOOLS)/tdrio

TDR_METACHECK_OBJS = tdr_meta_checker.o

TDR_IMPORT_LIBS = -Wl,-Bstatic $(TSF4G_LIB)/libtdr_xml.a   -Wl,-Bstatic -lscew -lexpat -Wl,-Bdynamic
TDR_IMPORT_LIBS_DBMS = $(TSF4G_LIB)/libtdr_dbms_xml.a    -Wl,-Bstatic -lscew -lexpat -Wl,-Bdynamic   $(TSF4G_DBMS_LIB)


.PHONY: all clean

all: $(TDR_METACHECK)

-include $(TDR_CPPFILE:.cpp=.d)

%.d: %.cpp
	$(CXX) -MM $(TDR_TOOLS_CFLAGS)  $< -o $@

%.o: %.cpp %.d
	$(CXX) -c $(TDR_TOOLS_CFLAGS) $<

$(TDR_METACHECK) : $(TDR_METACHECK_OBJS)
	$(CXX) -o $@ $(TDR_TOOLS_CFLAGS)    $^ $(LDPATH) $(TDR_IMPORT_LIBS_DBMS)

clean:
	$(RM) *.o  *.d
	$(RM) $(TDR_METACHECK)
