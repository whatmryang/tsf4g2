include ../../../tsf4g.mk

APP_CFILE=$(wildcard *.c)
APP_OBJ=$(APP_CFILE:.c=.o)
APP_LIBS=$(LDPATH) $(TSF4G_LIB)/libtqqsig.a $(TSF4G_LIB)/libtsf4g.a  -lscew -lexpat -lpthread

APP_EXE=tqqsigdemo

.PHONY: all clean


all: $(APP_EXE)

-include $(APP_CFILE:.c=.d)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $<

$(APP_EXE): $(APP_OBJ)
	$(CC)  -o $(APP_EXE) $(CFLAGS)  $^ $(APP_LIBS) 
	
clean:
	-$(RM) *.o  *.d
	-$(RM) $(APP_EXE)	


