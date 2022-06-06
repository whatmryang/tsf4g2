# @file $RCSfile: Makefile.st,v $
#  general description of this module
#  $Id: Makefile.st,v 1.5 2009-08-20 07:54:28 jacky Exp $
#  @author $Author: jacky $
#  @date $Date: 2009-08-20 07:54:28 $
#  @version 1.0
#  @note Editor: Vim 6.1, Gcc 4.0.2, tab=4
#  @note Platform: Linux

include ../../tsf4g.mk
include Makefile.pub


TDR_OBJ_DIR=$(TSF4G_HOME)/.obj/lib/tdr
TDR_LIB_OBJ_DIR=$(TDR_OBJ_DIR)/static

################################ libtdr.a ###################################
TDR_LIB_CFILE=$(wildcard *.c)
TDR_LIB_OBJ=$(addprefix $(TDR_LIB_OBJ_DIR)/,$(subst .c,.o, ${TDR_LIB_CFILE})) 
TDR_LIB_DEPS =  $(TDR_LIB_OBJ:.o=.d)
TDR_LIB=$(TSF4G_LIB)/libtdr.a


TDR_DBMS_WRAPPER_PATH = dbms
TDR_DBMS_WRAPPER_CFILE = $(wildcard $(TDR_DBMS_WRAPPER_PATH)/*.c)
TDR_DBMS_WRAPPER_OBJ =$(addprefix $(TDR_LIB_OBJ_DIR)/,$(subst .c,.o, ${TDR_DBMS_WRAPPER_CFILE}))  

TDR_XML_WRAPPER_PATH = xml_metalib
TDR_XML_WRAPPER_CFILE = $(wildcard $(TDR_XML_WRAPPER_PATH)/*.c)
TDR_XML_WRAPPER_OBJ = $(addprefix $(TDR_LIB_OBJ_DIR)/,$(subst .c,.o, ${TDR_XML_WRAPPER_CFILE}))  




TDR_CFLAGS = $(CFLAGS)  -I$(TSF4G_LIBSRC) -lgcc

MAKE_OBJ_PATH := $(shell mkdir -p $(TDR_OBJ_DIR) $(TDR_LIB_OBJ_DIR) \
	 $(TDR_LIB_OBJ_DIR)/$(TDR_DBMS_WRAPPER_PATH) $(TDR_LIB_OBJ_DIR)/$(TDR_XML_WRAPPER_PATH))


DEPS = $(TDR_LIB_OBJ:.o=.d) $(TDR_DBMS_WRAPPER_OBJ:.o=.d) $(TDR_XML_WRAPPER_OBJ:.o=.d)

 


TDR_LIB_WITH_XML=$(TSF4G_LIB)/libtdr_xml.a
TDR_LIB_WITH_DBMS_XML=$(TSF4G_LIB)/libtdr_dbms_xml.a

all: $(TDR_LIB) $(TDR_LIB_WITH_XML) $(TDR_LIB_WITH_DBMS_XML)
	@cp $(TDR_LIB_OBJ_DIR)/*.o  $(TSF4G_LIB_PUB_OBJ_DIR)
	@cp $(TDR_LIB_OBJ_DIR)/$(TDR_DBMS_WRAPPER_PATH)/*.o  $(TSF4G_LIB_PUB_OBJ_DIR)
	@cp $(TDR_LIB_OBJ_DIR)/$(TDR_XML_WRAPPER_PATH)/*.o  $(TSF4G_LIB_PUB_OBJ_DIR)

	


$(TDR_LIB_OBJ_DIR)/%.d: %.c
	$(CC)  $< $(TDR_CFLAGS)  -MM -MD -MQ'$(TDR_LIB_OBJ_DIR)/$*.o' -o $@

$(TDR_LIB_OBJ_DIR)/%.o: %.c $(TDR_LIB_OBJ_DIR)/%.d
	$(CC) -o $@ -c $< $(TDR_CFLAGS)
	

$(TDR_LIB):	$(TDR_LIB_OBJ)
	ar rsv $(TDR_LIB) $^ 
	


$(TDR_LIB_WITH_XML):	$(TDR_LIB_OBJ) $(TDR_XML_WRAPPER_OBJ)
	ar rsv $(TDR_LIB_WITH_XML) $(TDR_LIB_OBJ) $(TDR_XML_WRAPPER_OBJ)
	

$(TDR_LIB_WITH_DBMS_XML):	$(TDR_LIB_OBJ) $(TDR_DBMS_WRAPPER_OBJ) $(TDR_XML_WRAPPER_OBJ)
	ar rsv $(TDR_LIB_WITH_DBMS_XML) $(TDR_LIB_OBJ) $(TDR_DBMS_WRAPPER_OBJ) $(TDR_XML_WRAPPER_OBJ)
	
	
	

clean:
	$(RM) $(TDR_LIB_OBJ_DIR)/*.o  
	$(RM) $(TDR_LIB_OBJ_DIR)/*.d
	$(RM) $(TDR_LIB_OBJ_DIR)/$(TDR_XML_WRAPPER_PATH)/*.o  
	$(RM) $(TDR_LIB_OBJ_DIR)/$(TDR_XML_WRAPPER_PATH)/*.d  
	$(RM) $(TDR_LIB_OBJ_DIR)/$(TDR_DBMS_WRAPPER_PATH)/*.o
	$(RM) $(TDR_LIB_OBJ_DIR)/$(TDR_DBMS_WRAPPER_PATH)/*.d	
	$(RM) $(TDR_LIB)
	$(RM) $(TDR_LIB_WITH_DBMS_XML)
	$(RM) $(TDR_LIB_WITH_XML)

	
-include $(DEPS)

.PHONY: all clean
