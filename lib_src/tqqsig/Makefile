include ../../tsf4g.mk

APP_NAME=tqqsig

APP_CFILE=$(wildcard *.c)
APP_OBJ=$(APP_CFILE:.c=.o)
APP_LIB=$(TSF4G_LIB)/lib$(APP_NAME).a
APP_SO=$(TSF4G_LIB)/lib$(APP_NAME).so

.PHONY: all clean

all: $(APP_LIB) $(APP_SO)

-include $(APP_CFILE:.c=.d)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $(DEBUG) $<

$(APP_LIB):	$(APP_OBJ)
	ar r $(APP_LIB) $^
	
$(APP_SO): $(APP_OBJ)
	$(CC) -o $(APP_SO) $(SHARED) $(CFLAGS) $(DEBUG) $^	


clean:
	$(RM) *.o  *.d
	$(RM) $(APP_LIB) $(APP_SO)