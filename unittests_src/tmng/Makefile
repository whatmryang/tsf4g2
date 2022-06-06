include ../../tsf4g.mk


BIN=ut_tmng

SRCFILES:=$(wildcard *.c)

OBJFILES=$(subst .c,.o,$(SRCFILES))

#TESTOBJS= CUnitTest.o test_mib.o

#SRCOBJS= ../tmib.o ../tmetabase.o

SYSLIBPATH:=$(shell uname -m)
ifeq ($(SYSLIBPATH), x86_64)
	SYSLIBPATH=lib64
else
	SYSLIBPATH=lib
endif


MNG_TEST_LIBS= -ltmng -ltbus -ltlog -ltdr_xml -ltdr -lpal -lscew -lexpat -lcunit


all:$(BIN)

$(BIN):$(OBJFILES)
	$(CC) $(CFLAGS) -o $(BIN) $(OBJFILES) $(LDPATH) $(MNG_TEST_LIBS)
	cp $(BIN) $(TSF4G_UNITTESTS)/tmng/


$(OBJFILES):%.o:%.c
	$(CC) -c $(CFLAGS) -I../../lib_src/tmng $(SHARED) $(DEBUG) *.c

test:$(BIN)
	@ rm -f *.tdr
	@ for i in tdr_*.xml; do \
		i=$${i#tdr_}; \
		i=$${i%.xml}; \
		../../tools/tdr -B -o $${i}.tdr tdr_$${i}.xml >/dev/null 2>&1; \
	  done
	@ LD_LIBRARY_PATH="../../${SYSLIBPATH}:${LD_LIBRARY_PATH}" ./$(BIN)
	@ rm -f *.tdr

clean:
	rm -f *.o $(BIN)

