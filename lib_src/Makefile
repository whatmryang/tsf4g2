#  @file $RCSfile: Makefile,v $
#  general description of this module
#  $Id: Makefile,v 1.14 2009-09-07 10:47:48 vinsonzuo Exp $
#  @author $Author: vinsonzuo $
#  @date $Date: 2009-09-07 10:47:48 $
#  @version 1.0
#  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
#  @note Platform: Linux

include  ../tsf4g.mk
include  Makefile.pub
CLEANTARGET = clean

TSF4G_PACKLIB=$(TSF4G_LIB)/libtsf4g.a
TSF4G_PACKSO=libtsf4g.so

TSF4G_MT_PACKLIB=$(TSF4G_LIB)/libtsf4g_r.a
TSF4G_MT_PACKSO=libtsf4g_r.so

TSF4G_LIB_TMP_DIR = tmp

MODS_DEPENDON_TDRTOOL=  tlog tsec tbus tmng  tloghelp tqqsig
TSF4G_LIB_IMPORTLIB= $(LDPATH) $(TSF4G_DBMS_LIB) -lscew -lexpat


TSF4G_LIB_SO_RALATIVE=$(TSF4G_PACKSO).${TSF4G_LIB_MAJOR}.${TSF4G_LIB_MINOR}.$(TSF4G_LIB_PATCH)
TSF4G_LIB_SO_REAL=$(TSF4G_LIB)/$(TSF4G_LIB_SO_RALATIVE)
TSF4G_LIB_SO_NAME=$(TSF4G_PACKSO).${TSF4G_LIB_MAJOR}

TSF4G_MT_LIB_SO_RALATIVE=$(TSF4G_MT_PACKSO).${TSF4G_LIB_MAJOR}.${TSF4G_LIB_MINOR}.$(TSF4G_LIB_PATCH)
TSF4G_MT_LIB_SO_REAL=$(TSF4G_LIB)/$(TSF4G_MT_LIB_SO_RALATIVE)
TSF4G_MT_LIB_SO_NAME=$(TSF4G_MT_PACKSO).${TSF4G_LIB_MAJOR}

#ifeq ($(withsec), yes)
#	MODS_DEPENDON_TDRTOOL +=  tsec 
#	TSF4G_LIB_IMPORTLIB+=-lcrypto
#endif

TSF4G_LIB_IMPORTLIB+= $(LIBS)


MAKE_OBJ_PATH := $(shell mkdir -p $(TSF4G_LIB_PUB_OBJ_DIR) $(TSF4G_ST_LIB_OBJ_DIR) $(TSF4G_MT_LIB_OBJ_DIR))
	 
	 
MODS =pal comm  tdr taa  $(MODS_DEPENDON_TDRTOOL)

#ifeq ($(withobus), yes)
#	MODS += obus
#endif
MODS+=tapp


MODSCLEAN =$(patsubst %, %.$(CLEANTARGET), $(MODS))


all: $(MODS) tsf4glib

.PHONY:  all  clean $(MODS) $(MODSCLEAN) test 

$(MODS_DEPENDON_TDRTOOL):tdrtool

test:
	echo "Nothing to do "

tdrtool:
	@if test ! -f $(TSF4G_TOOLS)/tdr  ; then \
		echo "Begin to make tdr tools at first "; \
		cd $(TSF4G_TOOLSRC)/tdr && $(MAKE) ; \
		echo "Finish to make tdr tools  "; \
	fi;
	
$(MODS): 
	@echo $(withobus)
	@echo "Begin to make tsf4g module: '$@' ......" 
	cd $@ && $(MAKE);  
	@echo "Finish to  make tsf4g module: '$@'" 

	
tsf4glib:$(MODS)
	@echo "Begin to pack tsf4g library ......" 
	ar rsv $(TSF4G_PACKLIB) $(TSF4G_LIB_PUB_OBJ_DIR)/*.o $(TSF4G_ST_LIB_OBJ_DIR)/*.o 
	$(CC) -Wl,-soname,$(TSF4G_LIB_SO_NAME) -o $(TSF4G_LIB_SO_REAL) $(SHARED) $(CFLAGS)  $(TSF4G_LIB_PUB_OBJ_DIR)/*.o $(TSF4G_ST_LIB_OBJ_DIR)/*.o  $(TSF4G_LIB_IMPORTLIB)
	@$(LN) $(TSF4G_LIB) $(TSF4G_LIB_SO_RALATIVE) $(TSF4G_LIB_SO_NAME) 
	@$(LN) $(TSF4G_LIB) $(TSF4G_LIB_SO_NAME) $(TSF4G_PACKSO)



	@echo "Begin to pack tsf4g multithread library ......" 
	ar rsv $(TSF4G_MT_PACKLIB) $(TSF4G_LIB_PUB_OBJ_DIR)/*.o $(TSF4G_MT_LIB_OBJ_DIR)/*.o 
	$(CC) -Wl,-soname,$(TSF4G_MT_LIB_SO_NAME) -o $(TSF4G_MT_LIB_SO_REAL) $(SHARED) $(CFLAGS)  $(TSF4G_LIB_PUB_OBJ_DIR)/*.o $(TSF4G_MT_LIB_OBJ_DIR)/*.o  $(TSF4G_LIB_IMPORTLIB)
	@$(LN) $(TSF4G_LIB)  $(TSF4G_MT_LIB_SO_RALATIVE) $(TSF4G_MT_LIB_SO_NAME) 
	@$(LN) $(TSF4G_LIB) $(TSF4G_MT_LIB_SO_NAME) $(TSF4G_MT_PACKSO)
	@echo "Finish to pack tsf4g library ." 



	
largelib:
	@echo "Begin to pack tsf4g library ......" 
	ar rsv $(TSF4G_PACKLIB) $(TSF4G_LIB_PUB_OBJ_DIR)/*.o $(TSF4G_ST_LIB_OBJ_DIR)/*.o 
	$(CC) -Wl,-soname,$(TSF4G_LIB_SO_NAME) -o $(TSF4G_LIB_SO_REAL) $(SHARED) $(CFLAGS)  $(TSF4G_LIB_PUB_OBJ_DIR)/*.o $(TSF4G_ST_LIB_OBJ_DIR)/*.o  $(TSF4G_LIB_IMPORTLIB)
	@$(LN) $(TSF4G_LIB) $(TSF4G_LIB_SO_RALATIVE) $(TSF4G_LIB_SO_NAME) 
	@$(LN) $(TSF4G_LIB) $(TSF4G_LIB_SO_NAME) $(TSF4G_PACKSO)



	@echo "Begin to pack tsf4g multithread library ......" 
	ar rsv $(TSF4G_MT_PACKLIB) $(TSF4G_LIB_PUB_OBJ_DIR)/*.o $(TSF4G_MT_LIB_OBJ_DIR)/*.o 
	$(CC) -Wl,-soname,$(TSF4G_MT_LIB_SO_NAME) -o $(TSF4G_MT_LIB_SO_REAL) $(SHARED) $(CFLAGS)  $(TSF4G_LIB_PUB_OBJ_DIR)/*.o $(TSF4G_MT_LIB_OBJ_DIR)/*.o  $(TSF4G_LIB_IMPORTLIB)
	@$(LN) $(TSF4G_LIB)  $(TSF4G_MT_LIB_SO_RALATIVE) $(TSF4G_MT_LIB_SO_NAME) 
	@$(LN) $(TSF4G_LIB) $(TSF4G_MT_LIB_SO_NAME) $(TSF4G_MT_PACKSO)
	@echo "Finish to pack tsf4g library ." 


	
$(MODSCLEAN):
	@echo "Begin to clean tsf4g module: '$(patsubst %.$(CLEANTARGET),%, $@)' ......" 
	cd $(patsubst %.$(CLEANTARGET),%, $@) && $(MAKE) $(CLEANTARGET);
	@echo "Finish to clean tsf4g module: '$(patsubst %.$(CLEANTARGET),%, $@)'" 
	rm -rf ../.obj
tsf4glibclean:
	@$(RM) $(TSF4G_LIB_PUB_OBJ_DIR)/*
	@$(RM) $(TSF4G_ST_LIB_OBJ_DIR)/*
	@$(RM) $(TSF4G_MT_LIB_OBJ_DIR)/*
	
$(CLEANTARGET): $(MODSCLEAN)  tsf4glibclean
