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

TDR_TDR=$(TSF4G_TOOLS)/tdr

TDR_TDR_OBJS = $(patsubst %.cpp,%.o,$(TDR_CPPFILE)) tdr_tools.o tgetopt.o

TDR_IMPORT_LIBS = -Wl,-Bstatic $(TSF4G_LIB)/libtdr_xml.a -lpal  -Wl,-Bstatic -lscew -lexpat -Wl,-Bdynamic
TDR_IMPORT_LIBS_DBMS = $(TSF4G_LIB)/libtdr_dbms_xml.a  -lpal  -Wl,-Bstatic -lscew -lexpat -Wl,-Bdynamic   $(TSF4G_DBMS_LIB)


.PHONY: all clean

all: $(TDR_TDR)

-include $(TDR_CPPFILE:.cpp=.d)

tdr_tools.d: ../tdr_tools.cpp
	$(CXX) -MM $(TDR_TOOLS_CFLAGS)  $< -o $@

tgetopt.d: ../tgetopt.cpp
	$(CXX) -MM $(TDR_TOOLS_CFLAGS)  $< -o $@

tdr_tools.o: ../tdr_tools.cpp tdr_tools.d
	$(CXX) -c $(TDR_TOOLS_CFLAGS) $<

tgetopt.o: ../tgetopt.cpp tdr_tools.d
	$(CXX) -c $(TDR_TOOLS_CFLAGS) $<

%.d: %.cpp
	$(CXX) -MM $(TDR_TOOLS_CFLAGS)  $< -o $@

%.o: %.cpp %.d
	$(CXX) -c $(TDR_TOOLS_CFLAGS) $<

$(TDR_TDR): $(TDR_TDR_OBJS)
	$(CXX) -o $@  $(TDR_TOOLS_CFLAGS)   $^ $(LDPATH)  $(TDR_IMPORT_LIBS)

clean:
	$(RM) *.o  *.d
	$(RM) $(TDR_TDR)
